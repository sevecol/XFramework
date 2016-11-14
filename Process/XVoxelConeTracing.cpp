
#include "XVoxelConeTracing.h"
#include "..\DXSampleHelper.h"

#include "..\Resource\XTexture.h"
#include "..\Resource\XShader.h"
#include "..\Instance\XCamera.h"

extern XCamera									g_Camera;

extern XEngine *g_pEngine;
extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

extern D3D12_INPUT_ELEMENT_DESC PointElementDescs[];
extern UINT uPointElementCount;

namespace VoxelConeTracing
{
	UINT								uVoxelViewWorldW, uVoxelViewWorldH;
	UINT								uVoxelViewPixelW, uVoxelViewPixelH;
	float								fVoxelViewWPScale;

	UINT								uRenderTargetBase, uGpuCSUBase;
	XRenderTarget						*pRenderTargets[3] = { nullptr,nullptr,nullptr };
	D3D12_VIEWPORT						Viewport;
	D3D12_RECT							ScissorRect;
	
	XGraphicShader						*pGraphicShader = nullptr;

	struct FrameConstantBuffer
	{
		XMFLOAT4X4	mMv;
		XMFLOAT4X4	mMvp;				// Model-view-projection (MVP) matrix.
		XMFLOAT4X4	mMvpInv;			// Model-view-projection (MVP) matrix.
		XMFLOAT4	vEyePos;
		XMFLOAT4	vCameraNF;
		FLOAT		fPadding[8];
	};
	FrameConstantBuffer*				pFrameConstantBuffers;
	ComPtr<ID3D12Resource>				pFrameConstantUploadHeap;

	struct InstanceConstantBuffer
	{
		XMFLOAT4	vParameter;
		FLOAT		fPadding[60];
	};
	InstanceConstantBuffer*				pInstanceConstantBuffers;
	ComPtr<ID3D12Resource>				pInstanceConstantUploadHeap;
}
using namespace VoxelConeTracing;

extern D3D12_PRIMITIVE_TOPOLOGY_TYPE gTopologyType;
extern D3D12_FILL_MODE gFillMode;
bool InitVoxelConeTracing(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	uRenderTargetBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_RTV, 3);
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU, 5);

	//
	uVoxelViewPixelW = uVoxelViewPixelH = 32;
	uVoxelViewWorldW = uVoxelViewWorldH = 16;
	fVoxelViewWPScale = (float)uVoxelViewWorldW / (float)uVoxelViewPixelW;

	//
	Viewport.TopLeftX	= 0;
	Viewport.TopLeftY	= 0;
	Viewport.Width		= uVoxelViewPixelW;
	Viewport.Height		= uVoxelViewPixelH;
	Viewport.MaxDepth	= 1.0f;
	Viewport.MinDepth	= 0.0f;
	ScissorRect.left	= 0;
	ScissorRect.top		= 0;
	ScissorRect.right	= uVoxelViewPixelW;
	ScissorRect.bottom	= uVoxelViewPixelH;

	// RenderTarget
	for (unsigned int i = 0;i < 3;++i)
	{
		pRenderTargets[i] = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, uVoxelViewPixelW, uVoxelViewPixelH, uRenderTargetBase + i, uGpuCSUBase + i);
	}

	// GraphicShader
	DXGI_FORMAT uRenderTargetFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM };

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	gTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	gFillMode = D3D12_FILL_MODE_WIREFRAME;
	pGraphicShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shader_voxelconetracing.hlsl", XGraphicShaderInfo5("VSMain", "PSMain","GSMain"), depthStencilDesc, PointElementDescs, uPointElementCount,1, uRenderTargetFormat);
	gFillMode = D3D12_FILL_MODE_SOLID;
	gTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//
	// CB
	{
		// Create an upload heap for the constant buffers.
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(FrameConstantBuffer)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pFrameConstantUploadHeap)));

		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(pFrameConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pFrameConstantBuffers)));

		//
		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
		ConstantDesc.BufferLocation = pFrameConstantUploadHeap->GetGPUVirtualAddress();
		ConstantDesc.SizeInBytes = sizeof(VoxelConeTracing::FrameConstantBuffer);
		pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 3));
	}

	{
		// Create an upload heap for the constant buffers.
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(InstanceConstantBuffer)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pInstanceConstantUploadHeap)));

		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(pInstanceConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pInstanceConstantBuffers)));

		//
		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
		ConstantDesc.BufferLocation = pInstanceConstantUploadHeap->GetGPUVirtualAddress();
		ConstantDesc.SizeInBytes = sizeof(VoxelConeTracing::InstanceConstantBuffer);
		pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 4));

		//
		pInstanceConstantBuffers->vParameter.x = fVoxelViewWPScale;
		pInstanceConstantBuffers->vParameter.y = uVoxelViewWorldH / 2.0f - (0.5f*fVoxelViewWPScale);
	}

	return true;
}
void CleanVoxelConeTracing()
{
	// ConstantBuffer
	pFrameConstantUploadHeap->Unmap(0, nullptr);
	pFrameConstantBuffers = nullptr;
	pInstanceConstantUploadHeap->Unmap(0, nullptr);
	pInstanceConstantBuffers = nullptr;

	for (unsigned int i = 0;i < 3;++i)
	{
		SAFE_DELETE(pRenderTargets[i]);
	}
	XGraphicShaderManager::DelResource(&pGraphicShader);

	return;
}

void VoxelConeTracing_Update(UINT uIndex)
{
	XMFLOAT4X4 mv, mvp;

	XMMATRIX matView;
	XMMATRIX matProj = XMMatrixOrthographicLH(uVoxelViewWorldW, uVoxelViewWorldH, 1, 19);
	switch (uIndex)
	{
	case 0:
		matView = XMMatrixLookToLH(XMLoadFloat3(&XMFLOAT3(  0,   0, -10)), XMLoadFloat3(&XMFLOAT3( 0,  0,  1)), XMLoadFloat3(&XMFLOAT3(0, 1, 0)));
		break;
	case 1:
		matView = XMMatrixLookToLH(XMLoadFloat3(&XMFLOAT3(  0,  10,   0)), XMLoadFloat3(&XMFLOAT3( 0, -1,  0)), XMLoadFloat3(&XMFLOAT3(0, 0, 1)));
		break;
	case 2:
		matView = XMMatrixLookToLH(XMLoadFloat3(&XMFLOAT3( 10,   0,   0)), XMLoadFloat3(&XMFLOAT3(-1,  0,  0)), XMLoadFloat3(&XMFLOAT3(0, 1, 0)));
		break;
	}

	//
	//XMMATRIX temp = model * view;
	XMMATRIX temp = XMMatrixTranspose(matView);
	XMStoreFloat4x4(&mv, temp);
	pFrameConstantBuffers->mMv = mv;

	// Compute the model-view-projection matrix.
	temp = XMMatrixTranspose(matView * matProj);
	XMStoreFloat4x4(&mvp, temp);
	pFrameConstantBuffers->mMvp = mvp;

	//
	XMMATRIX tView = g_Camera.GetViewMatrix();
	tView.r[3].m128_f32[0] = 0.0f;
	tView.r[3].m128_f32[1] = 0.0f;
	tView.r[3].m128_f32[2] = 0.0f;
	temp = XMMatrixTranspose(tView * matProj);

	XMVECTOR det;
	XMMATRIX mvpinv = XMMatrixInverse(&det, temp);
	XMStoreFloat4x4(&mvp, mvpinv);
	pFrameConstantBuffers->mMvpInv = mvp;

	pFrameConstantBuffers->vEyePos.x = 0.0f;
	pFrameConstantBuffers->vEyePos.y = 0.0f;
	pFrameConstantBuffers->vEyePos.z = 0.0f;
	pFrameConstantBuffers->vEyePos.w = 1.0f;
	pFrameConstantBuffers->vCameraNF = XMFLOAT4(1.0f, 19.0f, 0.0f, 0.0f);

	return;
}
void VoxelConeTracing_Begin(ID3D12GraphicsCommandList *pCommandList, UINT uIndex)
{
	VoxelConeTracing_Update(uIndex);

	//
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTargets[uIndex]->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle(GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_RTV, uRenderTargetBase + uIndex));
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pCommandList->ClearRenderTargetView(RHandle, clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(DHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	pCommandList->RSSetViewports(1, &Viewport);
	pCommandList->RSSetScissorRects(1, &ScissorRect);

	//
	pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_FRAMEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 3));

	return;
}
void VoxelConeTracing_End(ID3D12GraphicsCommandList *pCommandList, UINT uIndex)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTargets[uIndex]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	return;
}

extern void RenderPoint(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void VoxelConeTracing_Render(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_INSTANCEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 4));
	pCommandList->SetGraphicsRootDescriptorTable(GRDT_SRV_TEXTURE, pRenderTargets[0]->GetSRVGpuHandle());
	RenderPoint(pCommandList, pGraphicShader);
	return;
}
