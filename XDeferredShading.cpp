#include "XDeferredShading.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#define DEFERREDSHADING_RENDERTARGET_RBASE		3
#define CSUBASE_DEFERREDSHADING_TEXTURE			3
#define DEFERREDSHADING_RENDERTARGET_COUNT		RENDERTARGET_MAXNUM

extern UINT										g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT								g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];
extern XEngine									*g_pEngine;

namespace DeferredShading
{
	XRenderTarget								*pRenderTargets[DEFERREDSHADING_RENDERTARGET_COUNT] = { nullptr,nullptr,nullptr };
	XShader										*pShadingShader;
	XComputeShader								*pClusteredShadingShader;

	//
	ID3D12Resource								*pResultBuffer = nullptr;
}
using namespace DeferredShading;

//
bool InitDeferredShading(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pRenderTargets[i] = XRenderTarget::CreateRenderTarget(g_RenderTargetFortmat[ESHADINGPATH_DEFERRED][i], uWidth, uHeight, DEFERREDSHADING_RENDERTARGET_RBASE + i, CSUBASE_DEFERREDSHADING_TEXTURE + i);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShadingShader = XShader::CreateShaderFromFile(L"shaders_ds_shading.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);
	pClusteredShadingShader = XComputeShader::CreateComputeShaderFromFile(L"shaders_ds_clusteredshading.hlsl", "CSMain", "cs_5_0");

	// ResultBuffer
/*
	ThrowIfFailed(pDevice->CreateCommittedResource(
	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
	D3D12_HEAP_FLAG_NONE,
	&CD3DX12_RESOURCE_DESC::Buffer(1280 * 720 * 4 * sizeof(float), D3D12_RESOURCE_FLAG_NONE),
	D3D12_RESOURCE_STATE_COPY_DEST,
	nullptr,
	IID_PPV_ARGS(&pResultBuffer)));
*/
	return true;
}

void CleanDeferredShading()
{
	SAFE_DELETE(pShadingShader);
	SAFE_DELETE(pClusteredShadingShader);
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		SAFE_DELETE(pRenderTargets[i]);
	}

	//
	SAFE_RELEASE(pResultBuffer);
}

void DeferredShading_GBuffer(ID3D12GraphicsCommandList* pCommandList)
{
	// Indicate that the back buffer will be used as a render target.
	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle[DEFERREDSHADING_RENDERTARGET_COUNT];
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTargets[i]->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		RHandle[i] = pRenderTargets[i]->GetRTVCpuHandle();
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

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
extern XRenderTarget* HDR_GetRenderTarget();
void DeferredShading_Shading(ID3D12GraphicsCommandList* pCommandList)
{
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTargets[i]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
	pCommandList->SetGraphicsRootDescriptorTable(2, pRenderTargets[0]->GetSRVGpuHandle());
	pCommandList->OMSetRenderTargets(0, nullptr, true, nullptr);

	//
	XRenderTarget* pHDRRenderTarget = HDR_GetRenderTarget();
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	
	//
	pCommandList->SetComputeRootSignature(g_pEngine->m_pComputeRootSignature.Get());
	pCommandList->SetPipelineState(pClusteredShadingShader->GetPipelineState());

	pCommandList->SetComputeRootDescriptorTable(0, pRenderTargets[0]->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(1, pHDRRenderTarget->GetUAVGpuHandle());

	//
	pCommandList->Dispatch(1280, 720, 1);

	//
/*
	// GetResult
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	pCommandList->CopyResource(pResultBuffer, pHDRRenderTarget->GetResource());
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	float *pAddress = nullptr;
	CD3DX12_RANGE readRange(0, 1280 * 720 * 4 * sizeof(float));
	pResultBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pAddress));
	//float fValue = *pAddress;

	float fValue = 0.0f;
	//for (UINT i = 0;i < g_uDispatchX * g_uDispatchY;++i)
	{
		fValue += pAddress[0];
	}
	pResultBuffer->Unmap(0, nullptr);
*/
	//
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &pHDRRenderTarget->GetRTVCpuHandle(), FALSE, &DHandle);

	//
	//RenderFullScreen(pCommandList, pShadingShader);
}