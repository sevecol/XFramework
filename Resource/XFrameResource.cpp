
#include "XFrameResource.h"
#include "..\DXSampleHelper.h"

extern ComPtr<ID3D12DescriptorHeap>		g_pRtvHeap;
extern ComPtr<ID3D12DescriptorHeap>		g_pDsvHeap;
extern UINT								g_uRtvDescriptorSize;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XFrameResource::~XFrameResource()
{
}

void XFrameResource::InitInstance(UINT uIndex, ID3D12Device* pDevice, IDXGISwapChain3 *pSwapChain)
{
	m_uIndex = uIndex;

	//
	ThrowIfFailed(pSwapChain->GetBuffer(m_uIndex, IID_PPV_ARGS(&m_pRenderTargets)));
	pDevice->CreateRenderTargetView(m_pRenderTargets.Get(), nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(),m_uIndex, g_uRtvDescriptorSize));

	//
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pRenderCommandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pRenderCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_pCommandList->Close());
}

extern D3D12_VIEWPORT						g_Viewport;
extern D3D12_RECT							g_ScissorRect;
void XFrameResource::PreRender()
{
	// Record all the commands we need to render the scene into the command list.
	// Create the command list.
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_pRenderCommandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_pCommandList->Reset(m_pRenderCommandAllocator.Get(), m_pPipelineState.Get()));

	m_pCommandList->RSSetViewports(1, &g_Viewport);
	m_pCommandList->RSSetScissorRects(1, &g_ScissorRect);
}

void XFrameResource::BeginRender()
{
	// Indicate that the back buffer will be used as a render target.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_uIndex, g_uRtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(g_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 1.2f, 0.4f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_pCommandList->ClearDepthStencilView(g_pDsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void XFrameResource::EndRender()
{
	// Indicate that the back buffer will now be used to present.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_pCommandList->Close());
}