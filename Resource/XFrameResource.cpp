
#include "XFrameResource.h"
#include "..\DXSampleHelper.h"

#define FRAMERESOURCE_RENDERTARGET_RBASE	0
#define FRAMERESOURCE_CONSTANT_CSUBASE		0
#define SHADING_RENDERTARGET_COUNT			1

extern ComPtr<ID3D12DescriptorHeap>		g_pRDescriptorHeap;
extern ComPtr<ID3D12DescriptorHeap>		g_pDDescriptorHeap;
extern UINT								g_uRDescriptorSize;

extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

extern ComPtr<ID3D12RootSignature>		g_pRootSignature;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XFrameResource::~XFrameResource()
{
	m_pConstantUploadHeap->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;
}

void XFrameResource::InitInstance(UINT uIndex, ID3D12Device* pDevice, IDXGISwapChain3 *pSwapChain)
{
	m_uIndex = uIndex;
	m_uFenceValue = 1;

	//
	ThrowIfFailed(pSwapChain->GetBuffer(m_uIndex, IID_PPV_ARGS(&m_pRenderTargets)));
	pDevice->CreateRenderTargetView(m_pRenderTargets.Get(), nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FRAMERESOURCE_RENDERTARGET_RBASE+m_uIndex, g_uRDescriptorSize));

	//
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pRenderCommandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pRenderCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_pCommandList->Close());
		
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	ThrowIfFailed(m_pConstantUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
	ConstantDesc.BufferLocation = m_pConstantUploadHeap->GetGPUVirtualAddress();
	ConstantDesc.SizeInBytes = sizeof(XFrameResource::ConstantBuffer);
	pDevice->CreateConstantBufferView(&ConstantDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FRAMERESOURCE_CONSTANT_CSUBASE+m_uIndex, g_uCSUDescriptorSize));
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
	ThrowIfFailed(m_pCommandList->Reset(m_pRenderCommandAllocator.Get(), nullptr));//m_pPipelineState.Get()));// ));

	m_pCommandList->SetGraphicsRootSignature(g_pRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { g_pCSUDescriptorHeap.Get() };
	m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_uIndex, g_uCSUDescriptorSize));

	m_pCommandList->RSSetViewports(1, &g_Viewport);
	m_pCommandList->RSSetScissorRects(1, &g_ScissorRect);
}

void XFrameResource::BeginRender()
{
	// Indicate that the back buffer will be used as a render target.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_uIndex, g_uRDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_pCommandList->OMSetRenderTargets(1, &RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pCommandList->ClearRenderTargetView(RHandle, clearColor, 0, nullptr);
	m_pCommandList->ClearDepthStencilView(g_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void XFrameResource::EndRender()
{
	// Indicate that the back buffer will now be used to present.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_pCommandList->Close());
}

void XM_CALLCONV XFrameResource::UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection)
{
	XMMATRIX model;
	XMFLOAT4X4 mvp;

	model = XMMatrixIdentity();
	//model = XMLoadFloat4x4(&m_modelMatrices[i * m_cityColumnCount + j]);

	// Compute the model-view-projection matrix.
	XMStoreFloat4x4(&mvp, XMMatrixTranspose(model * view * projection));

	// Copy this matrix into the appropriate location in the upload heap subresource.
	//m_pConstantBuffers[0].mvp._11 = 1.5f;
	//ZeroMemory(&m_pConstantBuffers[0], sizeof(ConstantBuffer));
	memcpy(&m_pConstantBuffers[0], &mvp, sizeof(mvp));
	//m_pConstantBuffers[0].mvp._11 = 0.5f;
	//m_pConstantBuffers[0].mvp._22 = 0.5f;
	//m_pConstantBuffers[0].mvp._33 = 0.5f;
	//m_pConstantBuffers[0].mvp._44 = 0.5f;
}