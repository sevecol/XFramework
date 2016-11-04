
#include "XSSAO.h"
#include "..\DXSampleHelper.h"
#include "..\Instance\XCamera.h"

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

GFSDK_SSAO_RenderTargetView_D3D12 mColorRTV[FRAME_NUM];
namespace SSAO
{
	GFSDK_SSAO_RenderTargetView_D3D12 mNormalRTV[FRAME_NUM];
	GFSDK_SSAO_ShaderResourceView_D3D12 mNormalSRV[FRAME_NUM];

	// Color buffer and render target
	//GFSDK_SSAO_RenderTargetView_D3D12 mColorRTV[FRAME_NUM];

	// HBAO+ context and parameter
	GFSDK_SSAO_Context_D3D12* mSSAO;
	GFSDK_SSAO_Parameters mAOParams;

	ComPtr<ID3D12DescriptorHeap> mSsaoDescHeapCbvSrvUav;
}
using namespace SSAO;

//
void CHK(HRESULT hr)
{
	//if (FAILED(hr))
	//	throw runtime_error("HRESULT is failed value.");
}
bool InitSSAO(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	//
	const UINT NodeMask = 1;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = SSAO_NUM_SRV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = NodeMask;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mSsaoDescHeapCbvSrvUav.ReleaseAndGetAddressOf())));
	mSsaoDescHeapCbvSrvUav->SetName(L"ViewerSsaoDescHeapCbvSrvUav");

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

	CD3DX12_CPU_DESCRIPTOR_HANDLE DepthSRV(mSsaoDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
	g_pEngine->m_pDevice->CreateShaderResourceView(g_pEngine->m_pDepthStencil.Get(), &depthSRVDesc, DepthSRV);

	//
	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps;

	DescriptorHeaps.CBV_SRV_UAV.pDescHeap = mSsaoDescHeapCbvSrvUav.Get();
	DescriptorHeaps.CBV_SRV_UAV.BaseIndex = SSAO_NUM_DEPTH_SRV;
	DescriptorHeaps.CBV_SRV_UAV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12;

	DescriptorHeaps.RTV.pDescHeap = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_pDescriptorHeap.Get();
	DescriptorHeaps.RTV.BaseIndex = APP_NUM_RTV;
	DescriptorHeaps.RTV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;

	GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12(g_pEngine->m_pDevice, NodeMask, DescriptorHeaps, &mSSAO, &CustomHeap);
	assert(status == GFSDK_SSAO_OK);

	mAOParams = {};
	mAOParams.Radius = 2.f;
	mAOParams.Bias = 0.2f;
	mAOParams.PowerExponent = 2.f;
	mAOParams.Blur.Enable = true;
	mAOParams.Blur.Sharpness = 32.f;
	mAOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;

	return true;
}
void CleanSSAO()
{
	if (mSSAO)
	{
		mSSAO->Release();
		mSSAO = nullptr;
	}
}

void SSAO_Render(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pEngine->m_pDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	//if (gUseSSAO)
	{
		// Set input data
		GFSDK_SSAO_InputData_D3D12 InputData = {};
		InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;

		// FullResDepthTextureSRV
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE DepthSrvGpuHandle(
				mSsaoDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
			InputData.DepthData.FullResDepthTextureSRV.pResource = g_pEngine->m_pDepthStencil.Get();
			InputData.DepthData.FullResDepthTextureSRV.GpuHandle = DepthSrvGpuHandle.ptr;
		}

		// DepthData
		XMMATRIX ProjMat = g_Camera.GetProjectionMatrix();
		InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&ProjMat);
		InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

#if USE_BIN_MESH_READER
		InputData.DepthData.MetersToViewSpaceUnits = 0.005f;
#else
		InputData.DepthData.MetersToViewSpaceUnits = 1.f;
#endif

		// NormalData
		{
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
		}

		//GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_DEBUG_NORMAL;
		GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;

		// Set SSAO descriptor heap
		{
			ID3D12DescriptorHeap* descHeaps[] = { mSsaoDescHeapCbvSrvUav.Get() };
			pCommandList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
		}

		GFSDK_SSAO_Output_D3D12 Output;
		Output.pRenderTargetView = &mColorRTV[g_uFrameIndex];

		GFSDK_SSAO_Status status = mSSAO->RenderAO(g_pEngine->m_pRenderCommandQueue.Get(), pCommandList, InputData, mAOParams, Output, RenderMask);
		assert(status == GFSDK_SSAO_OK);

		// Revert to the original descriptor heap
		{
			//ID3D12DescriptorHeap* descHeaps[] = { mDescHeapCbvSrvUav.Get() };
			//mCmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
		}

		//
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pEngine->m_pDepthStencil.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}
}

ID3D12DescriptorHeap* GetSSAODescriptorHeap()
{
	return mSsaoDescHeapCbvSrvUav.Get();
}