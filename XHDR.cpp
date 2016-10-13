#include "XHDR.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

extern XEngine							*g_pEngine;

XShader*								g_pHDRShader		= nullptr;
XRenderTarget							*g_pHDRRenderTarget = nullptr;

//
bool InitHDR(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	g_pHDRRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, uWidth, uHeight, 6, 9);

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	DXGI_FORMAT Format[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	g_pHDRShader = XShader::CreateShaderFromFile(L"shaders_hdr.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3,1, Format);

	return true;
}

void CleanHDR()
{
	SAFE_DELETE(g_pHDRRenderTarget);
	SAFE_DELETE(g_pHDRShader);
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

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader);
void HDR_ToneMaping(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pHDRRenderTarget->GetSRVGpuHandle());//CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 9, g_uCSUDescriptorSize));

	//
	RenderFullScreen(pCommandList, g_pHDRShader);
}