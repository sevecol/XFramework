
#include "XEntity.h"
#include "d3dx12.h"
#include "DXSampleHelper.h"
#include "Math\XMathSIMD.h"

#define ENTITY_TEXTURE_CSUBASE			9

extern ComPtr<ID3D12Device>				g_pDevice;
extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

extern XResourceThread					g_ResourceThread;

struct sVertex
{
	Vector3f	m_vPos;
	Vector2f	m_vTex;
	UINT32		m_uIndex;
	Vector4f	m_vWeight;
};
sVertex *pData = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XEntity::XEntity():m_pTextureSet(nullptr),m_pShader(nullptr),m_pGeometry(nullptr){}
XEntity::~XEntity()
{
	SAFE_DELETE(m_pTextureSet);
	SAFE_DELETE(m_pShader);
	SAFE_DELETE(m_pGeometry);
}

//
/*
void XEntity::SetResourceCheck(UINT uResourceCheck)
{
}
void XEntity::IncreaseResourceComplate()
{
}

void XEntity::Update(UINT32 deltatime)
{
	//Instance::Update(deltatime);
	//m_SkeletonAnimateControl.Update(deltatime, m_pGeometry);
}

IntersectionResult XEntity::Update(const OptFrustum* const frustum, UINT32 deltatime)
{
	return IR_Inside;
}
*/
void XEntity::Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	if ((pCommandList)&&(m_pShader->GetPipelineState()))
	{
/*
		TextureManager *pTextureManager = g_pXEngine->GetTextureManager();

		//
		//ID3D12DescriptorHeap* ppHeaps[] = { pTextureManager->GetSrvHeap() };
		//pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		pCommandList->SetGraphicsRootDescriptorTable(0, pTextureManager->GetGpuHangle(m_pTexture->GetSrvIndex()));
		pCommandList->SetPipelineState(m_pShader->GetPipelineState());

		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->IASetVertexBuffers(0, 1, m_pGeometry->GetVertexBufferView());
		if (m_pGeometry->GetNumIndices())
		{
			pCommandList->IASetIndexBuffer(m_pGeometry->GetIndexBufferView());
			pCommandList->DrawIndexedInstanced(m_pGeometry->GetNumIndices(), 1, 0, 0, 0);
		}
		else
		{
			pCommandList->DrawInstanced(3, 1, 0, 0);
		}
*/
		//
/*
		ID3D12DescriptorHeap* ppHeaps[] = { GetXEngine()->GetSceneGraph()->GetDescriptorHeap() };
		pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCommandList->SetGraphicsRootDescriptorTable(1, GetXEngine()->GetSceneGraph()->GetGpuHangle(m_uIndex));

		//
		TextureManager *pTextureManager = GetXEngine()->GetTextureManager();
		ppHeaps[0] = pTextureManager->GetDescriptorHeap();
		pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCommandList->SetGraphicsRootDescriptorTable(2, pTextureManager->GetGpuHangle(m_pTexture->GetSrvIndex()));
*/
		ID3D12DescriptorHeap *ppHeaps[] = { g_pCSUDescriptorHeap.Get() };
		pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCommandList->SetGraphicsRootDescriptorTable(2, CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), ENTITY_TEXTURE_CSUBASE, g_uCSUDescriptorSize));

		//
		pCommandList->SetPipelineState(m_pShader->GetPipelineState());
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->IASetVertexBuffers(0, 1, m_pGeometry->GetVertexBufferView());
		if (m_pGeometry->GetNumIndices())
		{
			pCommandList->IASetIndexBuffer(m_pGeometry->GetIndexBufferView());
			pCommandList->DrawIndexedInstanced(m_pGeometry->GetNumIndices(), 1, 0, 0, 0);
		}
		else
		{
			pCommandList->DrawInstanced(3, 1, 0, 0);
		}
	}

	//
	m_uFenceValue = uFenceValue;
}
bool XEntity::InitShader(LPCWSTR pFileName,LPCSTR pVSEntryPoint,LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[],UINT uInputElementCount)
{
	m_pShader = CreateShaderFromFile(pFileName, pVSEntryPoint, pVSTarget, pPSEntryPoint, pPSTarget, InputElementDescs, uInputElementCount, ESHADINGPATH_DEFERRED);
	return true;
}
/*
bool Entity::InitMaterial(LPCWSTR pName, UINT uWidth,UINT uHeight,UINT uPixelSize, CreateTextureFun pFun, UINT uParameter)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_pTexture = GetXEngine()->GetTextureManager()->CreateTexture(pName, uWidth, uHeight, uPixelSize, pFun, uParameter,this);
	return true;
}
*/
bool XEntity::InitTexture(UINT uCount, LPCWSTR pDetailName[])
{
	m_pTextureSet = new XTextureSet(ENTITY_TEXTURE_CSUBASE);
	//GetXEngine()->GetTextureManager()->CreateTextureFromFile(pFileName, uCount, pDetailName, this);

	//
	TextureSetLoad *pTextureSetLoad = new TextureSetLoad();
	pTextureSetLoad->m_pTextureSet = m_pTextureSet;
	pTextureSetLoad->m_pFun = nullptr;
	pTextureSetLoad->m_uParameter = 0;
	//pTextureSetLoad->m_pResourceSet = nullptr;//pResourceSet;

	for (UINT i = 0;i < uCount;++i)
	{
		STextureLayer sTextureLayer;
		sTextureLayer.m_sFileName = pDetailName[i];
		pTextureSetLoad->m_vTextureLayer.push_back(sTextureLayer);
	}

	g_ResourceThread.InsertResourceLoadTask(pTextureSetLoad);

	return true;
}

bool XEntity::InitGeometry(LPCWSTR pName, UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData, UINT uBoneCount, UINT8* pBoneIndex)
{
/*
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(g_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantBufferUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	ThrowIfFailed(m_pConstantBufferUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pConstantBufferUploadHeap->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(ConstantBuffer);
	g_pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), 3, g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
*/
	//
	m_pGeometry = CreateGeometry(uVertexCount, uVertexStride, uIndexCount, uIndexFormat, pGeometryData); 
	if (m_pGeometry)
	{
		//m_AxisAlignedBoundingBox.Add(m_pGeometry->m_vMin);
		//m_AxisAlignedBoundingBox.Add(m_pGeometry->m_vMax);
	}

	return true;
}
/*
bool Entity::InitSkeleton(LPCWSTR pFileName)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_SkeletonAnimateControl.m_pSkeleton = new Skeleton;//g_pXEngine->GetGeometryManager()->CreateGeometry(pName);
	if (m_SkeletonAnimateControl.m_pSkeleton)
	{
		GetXEngine()->GetResourceThread()->InsertSkeletonLoadTask(pFileName, dynamic_cast<Skeleton*>(m_SkeletonAnimateControl.m_pSkeleton), this);
	}

	return true;
}
bool Entity::InitAnimate(LPCWSTR pFileName)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_SkeletonAnimateControl.m_pAnimate = new Animate;//g_pXEngine->GetGeometryManager()->CreateGeometry(pName);
	if (m_SkeletonAnimateControl.m_pAnimate)
	{
		GetXEngine()->GetResourceThread()->InsertAnimateLoadTask(pFileName, dynamic_cast<Animate*>(m_SkeletonAnimateControl.m_pAnimate), this);
	}

	return true;
}

void Entity::PlayAnimate(UINT32 uAnimateID)
{
	m_SkeletonAnimateControl.PlayAnimate(uAnimateID);
}
*/