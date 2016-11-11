
#include "XPostProcess.h"

#include "..\..\Resource\XTexture.h"
#include "..\..\XHDR.h"

extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);

namespace PostProcess
{
	UINT										uRenderTargetBase, uGpuCSUBase;
	XRenderTarget								*pRenderTarget = nullptr;
}
using namespace PostProcess;

bool InitPostProcess(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	uRenderTargetBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_RTV, 1);
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU, 1);

	// RenderTarget
	{
		pRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, uWidth, uHeight, uRenderTargetBase, uGpuCSUBase, uGpuCSUBase);
	}

	return true;
}
void CleanPostProcess()
{
	SAFE_DELETE(pRenderTarget);
}

void PostProcess_Bind(ID3D12GraphicsCommandList* pCommandList)
{
	XRenderTarget* pHDRRenderTarget = HDR_GetRenderTarget();

	// Copy 
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	pCommandList->CopyResource(pRenderTarget->GetResource(), pHDRRenderTarget->GetResource());
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	pCommandList->SetGraphicsRootDescriptorTable(GRDT_SRV_POSTPROCESSTEXTURE, pRenderTarget->GetSRVGpuHandle());
}