
#include "XDeferredShading.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#include "Instance\XSkyBox.h"
#include "Instance\XCamera.h"

#define DEFERREDSHADING_RENDERTARGET_COUNT		RENDERTARGET_MAXNUM

extern XCamera									g_Camera;

extern UINT	g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];

extern XEngine *g_pEngine;
extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

namespace DeferredShading
{
	UINT										uRenderTargetBase,uGpuCSUBase;
	UINT										uDispatchX, uDispatchY;
	XRenderTarget								*pRenderTargets[DEFERREDSHADING_RENDERTARGET_COUNT] = { nullptr,nullptr,nullptr };
	XGraphicShader								*pShadingShader = nullptr;
	XComputeShader								*pClusteredShadingShader = nullptr;

	LightConstantBuffer							*pConstantBuffers = nullptr;
	ID3D12Resource								*pConstantUploadHeap = nullptr;

	//
	ID3D12Resource								*pResultBuffer = nullptr;
}
using namespace DeferredShading;

//
bool InitDeferredShading(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	uRenderTargetBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_RTV, DEFERREDSHADING_RENDERTARGET_COUNT);
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,4);

	//
	uDispatchX = uWidth / 32 + 1;
	uDispatchY = uHeight / 32 + 1;

	//
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pRenderTargets[i] = XRenderTarget::CreateRenderTarget(g_RenderTargetFortmat[ESHADINGPATH_DEFERRED][i], uWidth, uHeight, uRenderTargetBase + i, uGpuCSUBase + i);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShadingShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_ds_shading.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);
	pClusteredShadingShader = XComputeShaderManager::CreateComputeShaderFromFile(L"Media\\shaders_ds_clusteredshading.hlsl", "CSMain", "cs_5_0");

	// ConstantBuffer
	{
		// Create an upload heap for the constant buffers.
		// HDRConstant
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightConstantBuffer)),
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
		ConstantDesc.SizeInBytes = sizeof(LightConstantBuffer);
		pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase+3));
	}
/*
	// ResultBuffer
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
	// ConstantBuffer
	if (pConstantUploadHeap)
	{
		pConstantUploadHeap->Unmap(0, nullptr);
		SAFE_RELEASE(pConstantUploadHeap);
	}
	pConstantBuffers = nullptr;

	XGraphicShaderManager::DelResource(&pShadingShader);
	XComputeShaderManager::DelResource(&pClusteredShadingShader);
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
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(3, RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (UINT i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		pCommandList->ClearRenderTargetView(RHandle[i], clearColor, 0, nullptr);
	}
	pCommandList->ClearDepthStencilView(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
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
	pCommandList->SetPipelineState(pClusteredShadingShader->GetPipelineState());

	pCommandList->SetComputeRootDescriptorTable(0, pRenderTargets[0]->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(1, GetSkyBoxTexture()->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(2, pHDRRenderTarget->GetUAVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(5, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase +3));

	// For D3DWaring
	pCommandList->SetComputeRootDescriptorTable(3, pHDRRenderTarget->GetUAVGpuHandle());

	//
	pCommandList->Dispatch(uDispatchX, uDispatchY, 1);

	//
	// GetResult
/*
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
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &pHDRRenderTarget->GetRTVCpuHandle(), FALSE, &DHandle);

	//
	//RenderFullScreen(pCommandList, pShadingShader);
}

extern std::vector<PointLight> vPointLight;
void XM_CALLCONV DeferredShading_Update(FXMMATRIX view, CXMMATRIX projection)
{
	//
	XMFLOAT4X4 mMatrix;

	//
	XMMATRIX tView = view;
	//tView.r[3].m128_f32[0] = 0.0f;
	//tView.r[3].m128_f32[1] = 0.0f;
	//tView.r[3].m128_f32[2] = 0.0f;
	XMMATRIX temp = XMMatrixTranspose(tView);// *projection);
	//XMMATRIX temp = model * view * projection;//XMMatrixTranspose
	XMStoreFloat4x4(&mMatrix, temp);
	pConstantBuffers->mView = mMatrix;

	//
	temp = XMMatrixTranspose(projection);
	//temp = projection;
	XMStoreFloat4x4(&mMatrix, temp);
	pConstantBuffers->mProj = mMatrix;

	temp = XMMatrixTranspose(view*projection);
	//temp = projection;
	XMStoreFloat4x4(&mMatrix, temp);
	pConstantBuffers->mViewProj = mMatrix;

	//
	pConstantBuffers->vEyePos.x = g_Camera.GetPosition()->x;
	pConstantBuffers->vEyePos.y = g_Camera.GetPosition()->y;
	pConstantBuffers->vEyePos.z = g_Camera.GetPosition()->z;
	pConstantBuffers->vEyePos.w = 1.0f;
	pConstantBuffers->vCameraNF = XMFLOAT4(1.0f, 1000.0f,0.0f,0.0f);

	//
	pConstantBuffers->uLightNum = vPointLight.size();
	for (UINT i = 0;i < pConstantBuffers->uLightNum;++i)
	{
		//
		XMVECTOR wLightPos;
		wLightPos.m128_f32[0] = vPointLight[i].fPosX;
		wLightPos.m128_f32[1] = vPointLight[i].fPosY;
		wLightPos.m128_f32[2] = vPointLight[i].fPosZ;
		wLightPos.m128_f32[3] = 1.0f;
		XMVECTOR vLightPos0 = XMVector4Transform(wLightPos, tView);
		//XMVECTOR vLightPos1 = XMVector4Transform(wLightPos, view);
		pConstantBuffers->sLight[i].fPosX = wLightPos.m128_f32[0];
		pConstantBuffers->sLight[i].fPosY = wLightPos.m128_f32[1];
		pConstantBuffers->sLight[i].fPosZ = wLightPos.m128_f32[2];

		pConstantBuffers->sLight[i].fAttenuationBegin = vPointLight[i].fAttenuationBegin;
		pConstantBuffers->sLight[i].fAttenuationEnd = vPointLight[i].fAttenuationEnd;

		pConstantBuffers->sLight[i].fR = vPointLight[i].fR;
		pConstantBuffers->sLight[i].fG = vPointLight[i].fG;
		pConstantBuffers->sLight[i].fB = vPointLight[i].fB;
	}
}