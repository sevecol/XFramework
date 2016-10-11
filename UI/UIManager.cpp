
#include "UIImgWindow.h"
#include "UIManager.h"
#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

#include "..\Resource\XShader.h"
#include "..\Thread\XResourceThread.h"

UIManager					g_UIManager;
extern XResourceThread		g_ResourceThread;

UIManager::UIManager(): m_pShader(nullptr), m_pGeometry(nullptr)//
{
	m_pRootUIWindow = new UIWindow();
}
UIManager::~UIManager()
{
	SAFE_DELETE(m_pShader);
	SAFE_DELETE(m_pGeometry);

	if (m_pConstantBufferUploadHeap)
	{
		m_pConstantBufferUploadHeap->Unmap(0, nullptr);
	}
	for (UINT32 i = 0;i < UIWINDOWBUFF_NUM;++i)
	{
		m_pConstantBuffers[i] = nullptr;
	}
	SAFE_DELETE(m_pRootUIWindow);
}
/*
IUIWindow* UIManager::GetRootUIWindow()
{
	return m_pRootUIWindow;
}
*/
class BinResource : public IResourceLoad
{
public:
	UIManager *pUIManager;
	virtual void LoadFromFile()
	{
		//
		struct Vertex
		{
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 uv;
		};
		Vertex triangleVertices[] =
		{
			{ { -1.00f,  1.00f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
			{ {  1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },

			{ { -1.00f,  1.00f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ {  1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
			{ {  1.00f,  1.00f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
		};
		UINT uIndex[] = { 0,1,2,3,4,5 };

		UINT8 *pData = new UINT8[6 * sizeof(Vertex) + 6 * sizeof(UINT)];
		UINT8 *pVertexData = pData;
		memcpy(pVertexData, &triangleVertices[0], 6 * sizeof(Vertex));
		UINT8 *pIndexData = pData + 6 * sizeof(Vertex);
		memcpy(pIndexData, &uIndex[0], 6 * sizeof(UINT));

		XGeometry *pGeometry = CreateGeometry(6, sizeof(Vertex), 6, DXGI_FORMAT_R32_UINT, pData);//dynamic_cast<Geometry*>(GetXEngine()->GetGeometryManager()->CreateGeometry(L"UIGeometry"));
		if (pGeometry)
		{
/*
			if (pGeometry->GetRefCount()==1)
			{
				pGeometry->Init(6, sizeof(Vertex), 6, DXGI_FORMAT_R32_UINT, pData);
			}
*/
			pUIManager->SetGeometry(pGeometry);
		}
		delete[] pData;
	}
	virtual void PostLoad()
	{
		//pUIManager->IncreaseResourceComplate();
	}
	virtual bool IsNeedWaitForResource()
	{
		return true;
	}
};
/*
IUIWindow* UIManager::CreatePopUIWindow(UINT16 uX, UINT16 uY, UINT16 uWidth, UINT16 uHeight)
{
	return nullptr;
}
*/
UIWindow* UIManager::CreateUIImgWindow(UIWindow* pParent, LPCWSTR pImgFileName, UINT16 uX, UINT16 uY, UINT16 uWidth, UINT16 uHeight)
{
	UIImgWindow *pUIImgWindow = new UIImgWindow();
	pUIImgWindow->SetWidthHeight(uWidth, uHeight);
	//pUIImgWindow->SetResourceCheck(1);

	//LPCWSTR pDetailName[] = {L""};
	//pUIImgWindow->m_pTexture = GetXEngine()->GetTextureManager()->CreateTextureFromFile(pImgFileName, 1, pDetailName, pUIImgWindow);

	//
	UIWindow *pParentWnd = dynamic_cast<UIWindow*>(pParent);
	if (pParentWnd)
	{
		pParentWnd->AddChild(pUIImgWindow,uX,uY);
	}
	else
	{
		m_pRootUIWindow->AddChild(pUIImgWindow, uX, uY);
	}

	return pUIImgWindow;
}
void UIManager::DelUIWindow(UIWindow*& pUIWindow)
{
}
void UIManager::DelUIWindows()
{
}

void UIManager::Init(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	UINT uCountPerLayer[] = { UIWINDOWBUFF_NUM };
	//ResourceHeap::Init(pDevice, uCountPerLayer);
/*
	D3D12_DESCRIPTOR_HEAP_DESC ResourceHeapDesc = {};
	ResourceHeapDesc.NumDescriptors = UIWINDOWBUFF_NUM;
	ResourceHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ResourceHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&ResourceHeapDesc, IID_PPV_ARGS(&m_pResourceHeap)));

	m_uResourceHeapDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
*/
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)*UIWINDOWBUFF_NUM),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantBufferUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	ThrowIfFailed(m_pConstantBufferUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffers[0])));

	//
/*
	for (UINT32 i = 0;i < UIWINDOWBUFF_NUM;++i)
	{
		m_pConstantBuffers[i] = m_pConstantBuffers[0] + sizeof(ConstantBuffer) * i;

		//
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pConstantBufferUploadHeap->GetGPUVirtualAddress() + sizeof(ConstantBuffer) * i;
		cbvDesc.SizeInBytes = sizeof(ConstantBuffer);
		pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pResourceHeap.Get()->GetCPUDescriptorHandleForHeapStart(), i, pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
	}
*/
	//
	m_pRootUIWindow->SetWidthHeight(uWidth, uHeight);
	//SetResourceCheck(2);

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	m_pShader = CreateShaderFromFile(L"shaders_ui.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);

	BinResource *pbinresource = new BinResource();
	pbinresource->pUIManager = this;
	g_ResourceThread.InsertResourceLoadTask(pbinresource);
}

void UIManager::Update()
{
	//RenderList<UIWindow>::Clear();
	m_pRootUIWindow->Update();
}
void UIManager::Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	if ((pCommandList) && (m_pShader->GetPipelineState()))
	{
		pCommandList->SetPipelineState(m_pShader->GetPipelineState());
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//
		m_pRootUIWindow->Render(pCommandList, uFenceValue);
	}
}

bool UIManager::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pRootUIWindow->ProcessMessage(message, wParam, lParam);
}

void UIManager::SetGeometry(XGeometry *pGeometry)
{
	m_pGeometry = pGeometry;
}