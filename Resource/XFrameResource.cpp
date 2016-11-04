
#include "XFrameResource.h"
#include "XBuffer.h"
#include "..\DXSampleHelper.h"
#include "..\PostProcess\XSSAO.h"

#include "..\Instance\XCamera.h"
#include "..\Instance\XSkyBox.h"

#define SHADING_RENDERTARGET_COUNT			1

extern XCamera								g_Camera;

extern XEngine *g_pEngine;
extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

namespace FrameResource
{
	UINT									uRenderTargetBase,uGpuCSUBase;
}
using namespace FrameResource;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XFrameResource::~XFrameResource()
{
	m_pConstantUploadHeap->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;
}

void XFrameResource::Init(ID3D12Device* pDevice)
{
	uRenderTargetBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_RTV, 3);
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,3);
}

extern GFSDK_SSAO_RenderTargetView_D3D12 mColorRTV[FRAME_NUM];
void XFrameResource::InitInstance(UINT uIndex, ID3D12Device* pDevice, IDXGISwapChain3 *pSwapChain)
{
	m_uIndex = uIndex;
	m_uFenceValue = 1;

	//
	ThrowIfFailed(pSwapChain->GetBuffer(m_uIndex, IID_PPV_ARGS(&m_pRenderTargets)));

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_RTV, uRenderTargetBase + m_uIndex);
	pDevice->CreateRenderTargetView(m_pRenderTargets.Get(), nullptr, RenderTargetView);

	//
	mColorRTV[uIndex] = {};
	mColorRTV[uIndex].CpuHandle = RenderTargetView.ptr;
	mColorRTV[uIndex].pResource = m_pRenderTargets.Get();

	{
/*
		// SRV
		D3D12_SHADER_RESOURCE_VIEW_DESC NormalSRVDesc = {};
		NormalSRVDesc.Format = NormalRTVDesc.Format;
		NormalSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#if MSAA_SAMPLE_COUNT > 1
		NormalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
#else
		NormalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		NormalSRVDesc.Texture2D.MipLevels = 1;
		NormalSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
		NormalSRVDesc.Texture2D.PlaneSlice = 0;
		NormalSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
#endif

		mNormalSRV[FrameIndex] = {};
		mNormalSRV[FrameIndex].pResource = mNormalBuffer[FrameIndex].Get();
		CD3DX12_CPU_DESCRIPTOR_HANDLE NormalSRVHandle(
			mSsaoDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart(),
			SSAO_NUM_DEPTH_SRV + FrameIndex,
			mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mDev->CreateShaderResourceView(mNormalBuffer[FrameIndex].Get(), &NormalSRVDesc, NormalSRVHandle);
*/
	}

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
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_pConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
	ConstantDesc.BufferLocation = m_pConstantUploadHeap->GetGPUVirtualAddress();
	ConstantDesc.SizeInBytes = sizeof(XFrameResource::ConstantBuffer);
	pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase+m_uIndex));
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

	m_pCommandList->SetGraphicsRootSignature(g_pEngine->m_pGraphicRootSignature.Get());
	m_pCommandList->SetComputeRootSignature(g_pEngine->m_pComputeRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_GCSU) };
	m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_FRAMEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase+m_uIndex));
	m_pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_INSTANCEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase+m_uIndex));
	m_pCommandList->SetGraphicsRootDescriptorTable(GRDT_SRV_GLOBALTEXTURE, GetSkyBoxTexture()->GetSRVGpuHandle());

	m_pCommandList->RSSetViewports(1, &g_pEngine->m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &g_pEngine->m_ScissorRect);
}

void XFrameResource::BeginRender()
{
	// Indicate that the back buffer will be used as a render target.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle(GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_RTV, uRenderTargetBase+m_uIndex));
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart());
	m_pCommandList->OMSetRenderTargets(1, &RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pCommandList->ClearRenderTargetView(RHandle, clearColor, 0, nullptr);
	//m_pCommandList->ClearDepthStencilView(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void XFrameResource::EndRender()
{
	// Indicate that the back buffer will now be used to present.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_pCommandList->Close());
}

void XM_CALLCONV XFrameResource::UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection)
{
	XMFLOAT4X4 mv,mvp;

	//model = XMMatrixIdentity();
	//model = XMLoadFloat4x4(&m_modelMatrices[i * m_cityColumnCount + j]);

	//
	//XMMATRIX temp = model * view;
	XMMATRIX temp = XMMatrixTranspose(view);
	XMStoreFloat4x4(&mv, temp);
	m_pConstantBuffers->mMv = mv;

	// Compute the model-view-projection matrix.
	temp = XMMatrixTranspose(view * projection);
	XMStoreFloat4x4(&mvp, temp);
	m_pConstantBuffers->mMvp = mvp;

	//
	XMMATRIX tView = view;
	tView.r[3].m128_f32[0] = 0.0f;
	tView.r[3].m128_f32[1] = 0.0f;
	tView.r[3].m128_f32[2] = 0.0f;
	temp = XMMatrixTranspose(tView * projection);

	XMVECTOR det;
	XMMATRIX mvpinv = XMMatrixInverse(&det, temp);
	XMStoreFloat4x4(&mvp, mvpinv);
	m_pConstantBuffers->mMvpInv = mvp;
	//memcpy(&m_pConstantBuffers[0], &mvp, sizeof(mvp));
	//m_pConstantBuffers[0].mvp._11 = 0.5f;
	//m_pConstantBuffers[0].mvp._22 = 0.5f;
	//m_pConstantBuffers[0].mvp._33 = 0.5f;
	//m_pConstantBuffers[0].mvp._44 = 0.5f;

	m_pConstantBuffers->vEyePos.x = g_Camera.GetPosition()->x;
	m_pConstantBuffers->vEyePos.y = g_Camera.GetPosition()->y;
	m_pConstantBuffers->vEyePos.z = g_Camera.GetPosition()->z;
	m_pConstantBuffers->vEyePos.w = 1.0f;
	m_pConstantBuffers->vCameraNF = XMFLOAT4(g_Camera.GetNear(), g_Camera.GetFar(), 0.0f, 0.0f);
}