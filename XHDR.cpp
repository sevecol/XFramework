#include "XHDR.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

extern XEngine							*g_pEngine;
extern XResourceThread					*g_pResourceThread;

XShader									*g_pHDRShader		= nullptr;
XRenderTarget							*g_pHDRRenderTarget = nullptr;

XShader									*g_pHDRShaderScreen = nullptr;
XTextureSet								*g_pHDRTextureScreen= nullptr;

//
bool InitHDR(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	g_pHDRRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, uWidth, uHeight, 6, 9);

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	DXGI_FORMAT Format[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	g_pHDRShader = XShader::CreateShaderFromFile(L"shaders_hdr_tonemapping.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3,1, Format);

	Format[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	g_pHDRShaderScreen = XShader::CreateShaderFromFile(L"shaders_hdr_screen.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, 1, Format);

	//
	g_pHDRTextureScreen = new XTextureSet(10);
	//
	DDSTextureSetLoad *pTextureSetLoad = new DDSTextureSetLoad();
	pTextureSetLoad->m_pTextureSet = g_pHDRTextureScreen;
	pTextureSetLoad->m_pFun = nullptr;
	pTextureSetLoad->m_uParameter = 0;
	//pTextureSetLoad->m_pResourceSet = nullptr;//pResourceSet;

	for (UINT i = 0;i < 1;++i)
	{
		STextureLayer sTextureLayer;
		sTextureLayer.m_sFileName = L"hdr.dds";
		pTextureSetLoad->m_vTextureLayer.push_back(sTextureLayer);
	}

	g_pResourceThread->InsertResourceLoadTask(pTextureSetLoad);

	return true;
}

void CleanHDR()
{
	SAFE_DELETE(g_pHDRRenderTarget);
	SAFE_DELETE(g_pHDRShader);
	SAFE_DELETE(g_pHDRShaderScreen);
	SAFE_DELETE(g_pHDRTextureScreen);
}

void HDR_Bind(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &g_pHDRRenderTarget->GetRTVCpuHandle(), FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pCommandList->ClearRenderTargetView(g_pHDRRenderTarget->GetRTVCpuHandle(), clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void HDR_ToneMaping(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pHDRRenderTarget->GetSRVGpuHandle());//CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 9, g_uCSUDescriptorSize));

	//
	RenderFullScreen(pCommandList, g_pHDRShader);
}