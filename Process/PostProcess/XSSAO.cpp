
#include "XSSAO.h"
#include "..\..\DXSampleHelper.h"
#include "..\..\Instance\XCamera.h"

#include "..\..\Resource\XShader.h"
#include "..\..\Resource\XTexture.h"

// Library link for HBAO+
#ifdef _WIN64
#pragma comment(lib, "Lib\\GFSDK_SSAO_D3D12.win64.lib")
#else
#pragma comment(lib, "Lib\\GFSDK_SSAO_D3D12.win32.lib")
#endif

extern XCamera								g_Camera;

extern XEngine *g_pEngine;
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern UINT									g_uFrameIndex;

namespace SSAO
{
	GFSDK_SSAO_RenderTargetView_D3D12	NormalRTV[FRAME_NUM];
	GFSDK_SSAO_ShaderResourceView_D3D12 NormalSRV[FRAME_NUM];
	GFSDK_SSAO_RenderTargetView_D3D12	ColorRTV[FRAME_NUM];

	// Color buffer and render target
	//GFSDK_SSAO_RenderTargetView_D3D12 mColorRTV[FRAME_NUM];

	// HBAO+ context and parameter
	GFSDK_SSAO_Context_D3D12			*pSSAO;
	GFSDK_SSAO_Parameters				AOParams;

	ComPtr<ID3D12DescriptorHeap>		SsaoDescHeapCbvSrvUav;

	bool								bApply;
}
using namespace SSAO;

//
void CHK(HRESULT hr)
{
	//if (FAILED(hr))
	//	throw runtime_error("HRESULT is failed value.");
}
void InitSSAO(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	bApply = true;

	//
	const UINT NodeMask = 1;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = SSAO_NUM_SRV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = NodeMask;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(SsaoDescHeapCbvSrvUav.ReleaseAndGetAddressOf())));
	SsaoDescHeapCbvSrvUav->SetName(L"ViewerSsaoDescHeapCbvSrvUav");

	//
	D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};

	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#if MSAA_SAMPLE_COUNT > 1
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
#else
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
	depthSRVDesc.Texture2D.PlaneSlice = 0;
	depthSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
#endif

	CD3DX12_CPU_DESCRIPTOR_HANDLE DepthSRV(SsaoDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
	g_pEngine->m_pDevice->CreateShaderResourceView(g_pEngine->m_pDepthStencil.Get(), &depthSRVDesc, DepthSRV);

	//
	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps;

	DescriptorHeaps.CBV_SRV_UAV.pDescHeap = SsaoDescHeapCbvSrvUav.Get();
	DescriptorHeaps.CBV_SRV_UAV.BaseIndex = SSAO_NUM_DEPTH_SRV;
	DescriptorHeaps.CBV_SRV_UAV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12;

	DescriptorHeaps.RTV.pDescHeap = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_pDescriptorHeap.Get();
	DescriptorHeaps.RTV.BaseIndex = APP_NUM_RTV;
	DescriptorHeaps.RTV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;

	GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12(g_pEngine->m_pDevice, NodeMask, DescriptorHeaps, &pSSAO, &CustomHeap);
	assert(status == GFSDK_SSAO_OK);

	AOParams = {};
	AOParams.Radius = 1.f;
	AOParams.Bias = 0.2f;
	AOParams.PowerExponent = 2.f;
	AOParams.Blur.Enable = true;
	AOParams.Blur.Sharpness = 32.f;
	AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
}

void CleanSSAO()
{
	if (pSSAO)
	{
		pSSAO->Release();
		pSSAO = nullptr;
	}
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
extern XRenderTarget* HDR_GetRenderTarget();
void SSAO_Render(ID3D12GraphicsCommandList *pCommandList)
{
	if (!bApply)
	{
		return;
	}

	//
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pEngine->m_pDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// Set SSAO descriptor heap
	ID3D12DescriptorHeap* descHeaps[] = { SsaoDescHeapCbvSrvUav.Get() };
	pCommandList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);

	// Set input data
	GFSDK_SSAO_InputData_D3D12 InputData = {};
	InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;

	// FullResDepthTextureSRV
	CD3DX12_GPU_DESCRIPTOR_HANDLE DepthSrvGpuHandle(SsaoDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
	InputData.DepthData.FullResDepthTextureSRV.pResource = g_pEngine->m_pDepthStencil.Get();
	InputData.DepthData.FullResDepthTextureSRV.GpuHandle = DepthSrvGpuHandle.ptr;

	// DepthData
	XMMATRIX ProjMat = g_Camera.GetProjectionMatrix();
	InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&ProjMat);
	InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

#if USE_BIN_MESH_READER
	InputData.DepthData.MetersToViewSpaceUnits = 1.5f;
#else
	InputData.DepthData.MetersToViewSpaceUnits = 1.f;
#endif

	InputData.NormalData.Enable = false;

	/*
	CD3DX12_GPU_DESCRIPTOR_HANDLE NormalSrvGpuHandle(
	mSsaoDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart(),
	SSAO_NUM_DEPTH_SRV + mFrameIndex,
	mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	mNormalSRV[mFrameIndex].GpuHandle = NormalSrvGpuHandle.ptr;

#if USE_BIN_MESH_READER
	InputData.NormalData.Enable = false;
#else
	InputData.NormalData.Enable = true;
#endif

	if (InputData.NormalData.Enable)
	{
		InputData.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&ViewMat);
		InputData.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		InputData.NormalData.FullResNormalTextureSRV = mNormalSRV[mFrameIndex];
	}
	*/

	GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;

	GFSDK_SSAO_Output_D3D12 Output;
	Output.pRenderTargetView = &ColorRTV[g_uFrameIndex];
	Output.Blend.Mode = GFSDK_SSAO_MULTIPLY_RGB;

	GFSDK_SSAO_Status status = pSSAO->RenderAO(g_pEngine->m_pRenderCommandQueue.Get(), pCommandList, InputData, AOParams, Output, RenderMask);
	assert(status == GFSDK_SSAO_OK);

	//
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pEngine->m_pDepthStencil.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

ID3D12DescriptorHeap* GetSSAODescriptorHeap()
{
	return SsaoDescHeapCbvSrvUav.Get();
}

GFSDK_SSAO_RenderTargetView_D3D12* GetSSAORenderTargetView()
{
	return SSAO::ColorRTV;
}

void ApplySSAO(bool bApply)
{
	SSAO::bApply = bApply;
}