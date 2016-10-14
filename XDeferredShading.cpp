#include "XDeferredShading.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#define DEFERREDSHADING_RENDERTARGET_RBASE		3
#define DEFERREDSHADING_CONSTANT_CSUBASE		3
#define DEFERREDSHADING_RENDERTARGET_COUNT		RENDERTARGET_MAXNUM

extern UINT										g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT								g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];

extern XEngine									*g_pEngine;

XRenderTarget									*g_pDRRenderTargets[DEFERREDSHADING_RENDERTARGET_COUNT] = { nullptr,nullptr,nullptr };
XShader*										g_pDeferredShadingShader;

//
bool InitDeferredShading(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		g_pDRRenderTargets[i] = XRenderTarget::CreateRenderTarget(g_RenderTargetFortmat[ESHADINGPATH_DEFERRED][i], uWidth, uHeight, DEFERREDSHADING_RENDERTARGET_RBASE + i, DEFERREDSHADING_CONSTANT_CSUBASE + i);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	g_pDeferredShadingShader = XShader::CreateShaderFromFile(L"shaders_ds_shading.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);

	return true;
}

void CleanDeferredShading()
{
	SAFE_DELETE(g_pDeferredShadingShader);
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		SAFE_DELETE(g_pDRRenderTargets[i]);
	}
}

void DeferredShading_GBuffer(ID3D12GraphicsCommandList* pCommandList)
{
	// Indicate that the back buffer will be used as a render target.
	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle[DEFERREDSHADING_RENDERTARGET_COUNT];
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[i]->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		RHandle[i] = g_pDRRenderTargets[i]->GetRTVCpuHandle();
	}
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(3, RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ClearRenderTargetView(RHandle[i], clearColor, 0, nullptr);
	}
	pCommandList->ClearDepthStencilView(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DeferredShading_PrepareShading(ID3D12GraphicsCommandList* pCommandList)
{
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[i]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pDRRenderTargets[0]->GetSRVGpuHandle());
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void DeferredShading_Shading(ID3D12GraphicsCommandList* pCommandList)
{
	//
	RenderFullScreen(pCommandList, g_pDeferredShadingShader);
}