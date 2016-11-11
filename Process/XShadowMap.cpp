
#include "XShadowMap.h"
#include "..\DXSampleHelper.h"

#include "..\Instance\XCamera.h"

extern XCamera g_Camera;
extern XEngine *g_pEngine;
extern XResourceThread *g_pResourceThread;

extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

extern void CreateDefaultDepthBuffer(ID3D12Resource** ppResource, UINT uWidth, UINT uHeight, D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor);

//
namespace ShadowMap
{
	UINT								uGpuCSUBase;
	ComPtr<ID3D12Resource>				pShadowMap;
	ComPtr<ID3D12DescriptorHeap>		ShadowMapDescHeapDSV;
	D3D12_VIEWPORT						Viewport;
	D3D12_RECT							ScissorRect;

	struct ConstantBuffer
	{
		XMFLOAT4X4	mMv;
		XMFLOAT4X4	mMvp;				// Model-view-projection (MVP) matrix.
		XMFLOAT4X4	mMvpInv;			// Model-view-projection (MVP) matrix.
		XMFLOAT4	vEyePos;
		XMFLOAT4	vCameraNF;
		FLOAT		fPadding[8];
	};
	ConstantBuffer*						pConstantBuffers;
	ComPtr<ID3D12Resource>				pConstantUploadHeap;
}
using namespace ShadowMap;

bool InitShadowMap(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	//
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU, 2);

	Viewport.TopLeftX	= 0;
	Viewport.TopLeftY	= 0;
	Viewport.Width		= 1024;
	Viewport.Height		= 1024;
	Viewport.MaxDepth	= 1.0f;
	Viewport.MinDepth	= 0.0f;
	ScissorRect.left	= 0;
	ScissorRect.top		= 0;
	ScissorRect.right	= 1024;
	ScissorRect.bottom	= 1024;

	//
	D3D12_DESCRIPTOR_HEAP_DESC DHeapDesc = {};
	DHeapDesc.NumDescriptors = 1;
	DHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&DHeapDesc, IID_PPV_ARGS(&ShadowMapDescHeapDSV)));

	//
	CreateDefaultDepthBuffer(pShadowMap.ReleaseAndGetAddressOf(), 1024, 1024, ShadowMapDescHeapDSV->GetCPUDescriptorHandleForHeapStart());

	//
	D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};

	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
	depthSRVDesc.Texture2D.PlaneSlice = 0;
	depthSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	CD3DX12_CPU_DESCRIPTOR_HANDLE DepthSRV(GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase));
	g_pEngine->m_pDevice->CreateShaderResourceView(pShadowMap.Get(), &depthSRVDesc, DepthSRV);

	// CB
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pConstantUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(pConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
	ConstantDesc.BufferLocation = pConstantUploadHeap->GetGPUVirtualAddress();
	ConstantDesc.SizeInBytes = sizeof(ShadowMap::ConstantBuffer);
	pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 1));

	return true;
}

void CleanShadowMap()
{
}

void ShadowMap_Begin(ID3D12GraphicsCommandList *pCommandList)
{
	static bool bFirst = true;
	if (!bFirst)
	{
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pShadowMap.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}
	bFirst = false;
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(ShadowMapDescHeapDSV->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(0, nullptr, FALSE, &DHandle);
	pCommandList->ClearDepthStencilView(DHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	pCommandList->RSSetViewports(1, &Viewport);
	pCommandList->RSSetScissorRects(1, &ScissorRect);

	//
	pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_FRAMEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 1));
}
void ShadowMap_End(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pShadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE,D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void XM_CALLCONV ShadowMap_Update(FXMMATRIX view, CXMMATRIX projection)
{
	XMFLOAT4X4 mv, mvp;

	//model = XMMatrixIdentity();
	//model = XMLoadFloat4x4(&m_modelMatrices[i * m_cityColumnCount + j]);

	//
	//XMMATRIX temp = model * view;
	XMMATRIX temp = XMMatrixTranspose(view);
	XMStoreFloat4x4(&mv, temp);
	pConstantBuffers->mMv = mv;

	// Compute the model-view-projection matrix.
	temp = XMMatrixTranspose(view * projection);
	XMStoreFloat4x4(&mvp, temp);
	pConstantBuffers->mMvp = mvp;

	//
	XMMATRIX tView = view;
	tView.r[3].m128_f32[0] = 0.0f;
	tView.r[3].m128_f32[1] = 0.0f;
	tView.r[3].m128_f32[2] = 0.0f;
	temp = XMMatrixTranspose(tView * projection);

	XMVECTOR det;
	XMMATRIX mvpinv = XMMatrixInverse(&det, temp);
	XMStoreFloat4x4(&mvp, mvpinv);
	pConstantBuffers->mMvpInv = mvp;
	//memcpy(&m_pConstantBuffers[0], &mvp, sizeof(mvp));
	//m_pConstantBuffers[0].mvp._11 = 0.5f;
	//m_pConstantBuffers[0].mvp._22 = 0.5f;
	//m_pConstantBuffers[0].mvp._33 = 0.5f;
	//m_pConstantBuffers[0].mvp._44 = 0.5f;

	pConstantBuffers->vEyePos.x = g_Camera.GetPosition()->x;
	pConstantBuffers->vEyePos.y = g_Camera.GetPosition()->y;
	pConstantBuffers->vEyePos.z = g_Camera.GetPosition()->z;
	pConstantBuffers->vEyePos.w = 1.0f;
	pConstantBuffers->vCameraNF = XMFLOAT4(g_Camera.GetNear(), g_Camera.GetFar(), 0.0f, 0.0f);
}