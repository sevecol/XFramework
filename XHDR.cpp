
#include "XHDR.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"
#include "Resource\XBuffer.h"

#define DISPATCHX_MAX					100
#define DISPATCHY_MAX					100
#define DISPATCHNUM_MAX					(DISPATCHX_MAX*DISPATCHY_MAX)

extern XEngine *g_pEngine;
extern XResourceThread *g_pResourceThread;

extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

//
namespace HDR
{
	UINT								uRenderTargetBase,uGpuCSUBase, uCpuCSUBase;

	UINT								uDispatchX, uDispatchY, uPixelCount;
	UINT								uSrcIndex = 0;

	enum eSBufferType
	{
		ESBUFFERTYPE_1 = 0,
		ESBUFFERTYPE_2,

		ESBUFFERTYPE_COUNT
	};
	IStructuredBuffer					*pSBuffer[ESBUFFERTYPE_COUNT] = { nullptr,nullptr };
	CD3DX12_CPU_DESCRIPTOR_HANDLE		hUAVCpuHandle[ESBUFFERTYPE_COUNT];

	XRenderTarget						*pRenderTarget = nullptr;
	XGraphicShader						*pShaderToneMapping = nullptr;

	enum eLuminancePhase
	{
		ELUMINANCEPHASE_2DTO1D = 0,
		ELUMINANCEPHASE_1DTO0D,

		ELUMINANCEPHASE_COUNT
	};
	XComputeShader						*pShaderLuminance[ELUMINANCEPHASE_COUNT] = { nullptr,nullptr };

	//XTextureSet						*pScreenTexture		= nullptr;
	//XShader							*pScreenShader		= nullptr;

	HDRConstantBuffer					*pConstantBuffers = nullptr;
	ID3D12Resource						*pConstantUploadHeap = nullptr;

	ID3D12Resource *pResultBuffer		= nullptr;
}
using namespace HDR;

bool InitHDR(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	uRenderTargetBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_RTV, 1);
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,5);
	uCpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_CCSU,2);

	//
	uDispatchX = min(uWidth / 16, DISPATCHX_MAX);
	uDispatchY = min(uHeight / 16, DISPATCHY_MAX);
	uPixelCount = uWidth * uHeight;

	// RenderTarget
	{
		pRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, uWidth, uHeight, uRenderTargetBase, uGpuCSUBase, uGpuCSUBase +1);
	}
	// StructuredBuffer
	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		pSBuffer[i] = new XStructuredBuffer<float>(pDevice, DISPATCHNUM_MAX, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 2 + i));
		pSBuffer[i]->SetUAVGpuHandle(GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 2 + i));

		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = DISPATCHNUM_MAX;
		UDesc.Buffer.StructureByteStride = sizeof(float);
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		hUAVCpuHandle[i] = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_CCSU, uCpuCSUBase + i);
		pDevice->CreateUnorderedAccessView(pSBuffer[i]->GetResource(), nullptr, &UDesc, hUAVCpuHandle[i]);
	}
	// ConstantBuffer
	{
		// Create an upload heap for the constant buffers.
		// HDRConstant
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(HDRConstantBuffer)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pConstantUploadHeap)));

		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(pConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pConstantBuffers)));

		pConstantBuffers->uDispatchX = uDispatchX;
		pConstantBuffers->uDispatchY = uDispatchY;
		pConstantBuffers->uScreenX = uWidth;
		pConstantBuffers->uScreenY = uHeight;

		//
		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
		ConstantDesc.BufferLocation = pConstantUploadHeap->GetGPUVirtualAddress();
		ConstantDesc.SizeInBytes = sizeof(HDRConstantBuffer);
		pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 4));
	}

	// Shader
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	DXGI_FORMAT Format[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	pShaderToneMapping = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_hdr_tonemapping.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3,1, Format);
	pShaderLuminance[HDR::ELUMINANCEPHASE_2DTO1D] = XComputeShaderManager::CreateComputeShaderFromFile(L"Media\\shaders_hdr_luminance1.hlsl", "CSMain", "cs_5_0");
	pShaderLuminance[HDR::ELUMINANCEPHASE_1DTO0D] = XComputeShaderManager::CreateComputeShaderFromFile(L"Media\\shaders_hdr_luminance2.hlsl", "CSMain", "cs_5_0");

	//
	//Format[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//g_pHDRShaderScreen = XShader::CreateShaderFromFile(L"shaders_hdr_screen.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, 1, Format);

	// Texture
	//LPCWSTR lpTextureFileName[] = {L"hdr.dds"};
	//g_pHDRTextureScreen = XTextureSet::CreateTextureSet(L"HDRTexture", 1, lpTextureFileName, GCSUBASE_HDR+1);

	/*
	// ResultBuffer
	ThrowIfFailed(pDevice->CreateCommittedResource(
	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
	D3D12_HEAP_FLAG_NONE,
	&CD3DX12_RESOURCE_DESC::Buffer(DISPATCHNUM_MAX * sizeof(float), D3D12_RESOURCE_FLAG_NONE),
	D3D12_RESOURCE_STATE_COPY_DEST,
	nullptr,
	IID_PPV_ARGS(&pResultBuffer)));
	*/

	return true;
}

void CleanHDR()
{
	// ConstantBuffer
	if (pConstantUploadHeap)
	{
		pConstantUploadHeap->Unmap(0, nullptr);
		SAFE_RELEASE(pConstantUploadHeap);
	}
	pConstantBuffers = nullptr;

	// StructuredBuffer
	SAFE_RELEASE(pResultBuffer);
	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		SAFE_DELETE(pSBuffer[i]);
	}

	// RenderTarget
	SAFE_DELETE(pRenderTarget);

	// Shader
	XGraphicShaderManager::DelResource(&pShaderToneMapping);
	for (UINT i = 0;i < ELUMINANCEPHASE_COUNT;++i)
	{
		XComputeShaderManager::DelResource(&pShaderLuminance[i]);
	}
	//SAFE_DELETE(g_pShaderScreen);

	// Texture
	//XTextureSet::DeleteTextureSet(&g_pTextureScreen);
}

void HDR_Bind(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &pRenderTarget->GetRTVCpuHandle(), FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pCommandList->ClearRenderTargetView(pRenderTarget->GetRTVCpuHandle(), clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(GetHandleHeap(XEngine::XDESCRIPTORHEAPTYPE_DSV)->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	//
	pCommandList->SetComputeRootDescriptorTable(4, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 4));
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void HDR_Luminance(ID3D12GraphicsCommandList* pCommandList);
void HDR_ToneMapping(ID3D12GraphicsCommandList* pCommandList)
{
	//
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->SetGraphicsRootDescriptorTable(2, pRenderTarget->GetSRVGpuHandle());

	// ComputeLuminance
	HDR_Luminance(pCommandList);

	//
	D3D12_GPU_DESCRIPTOR_HANDLE hStart = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 2 + uSrcIndex - 1);
	pCommandList->SetGraphicsRootDescriptorTable(3, hStart);
	pCommandList->SetGraphicsRootDescriptorTable(1, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 4));

	// ToneMapping
	RenderFullScreen(pCommandList, pShaderToneMapping);
}

void HDR_Luminance(ID3D12GraphicsCommandList* pCommandList)
{
	// ComputeShader
	// 2D to 1D
	FLOAT fClearValue[] = { 0.0f,0.0f, };
	pCommandList->ClearUnorderedAccessViewFloat(pSBuffer[0]->GetUAVGpuHandle(), hUAVCpuHandle[0], pSBuffer[0]->GetResource(), &fClearValue[0], 0, nullptr);

	//
	pCommandList->SetPipelineState(pShaderLuminance[ELUMINANCEPHASE_2DTO1D]->GetPipelineState());

	pCommandList->SetComputeRootDescriptorTable(0, pRenderTarget->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(2, pSBuffer[0]->GetUAVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(3, pSBuffer[1]->GetUAVGpuHandle());

	//
	pCommandList->Dispatch(uDispatchX, uDispatchY, 1);

	// 1D to Float
	pCommandList->SetPipelineState(pShaderLuminance[ELUMINANCEPHASE_1DTO0D]->GetPipelineState());

	UINT uPixeclCompute = uDispatchX * uDispatchY;
	UINT uDispatchX2Pass = static_cast<int>(ceil(uPixeclCompute / 128.f));

	HDR::uSrcIndex = 0;
	for (;;)
	{
		UINT uDestIndex = 1 - uSrcIndex;

		FLOAT fClearValue[] = { 0.0f,0.0f, };
		pCommandList->ClearUnorderedAccessViewFloat(pSBuffer[uDestIndex]->GetUAVGpuHandle(), hUAVCpuHandle[uDestIndex], pSBuffer[uDestIndex]->GetResource(), &fClearValue[0], 0, nullptr);

		//
		pCommandList->SetComputeRootDescriptorTable(2, pSBuffer[ uSrcIndex]->GetUAVGpuHandle());
		pCommandList->SetComputeRootDescriptorTable(3, pSBuffer[uDestIndex]->GetUAVGpuHandle());

		pCommandList->Dispatch(uDispatchX2Pass, 1, 1);

		//
		uSrcIndex = 1 - uSrcIndex;
		uPixeclCompute = uDispatchX2Pass;
		uDispatchX2Pass = static_cast<int>(ceil(uPixeclCompute / 128.f));

		if (uPixeclCompute == 1)
		{
			break;
		}
	}
/*
	// GetResult
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRSBuffer[uSrcIndex]->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	pCommandList->CopyResource(pResultBuffer, g_pHDRSBuffer[uSrcIndex]->GetResource());
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRSBuffer[uSrcIndex]->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	float *pAddress = nullptr;
	CD3DX12_RANGE readRange(0, g_uDispatchX * g_uDispatchY * sizeof(float));
	pResultBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pAddress));
	//float fValue = *pAddress;

	float fValue = 0.0f;
	//for (UINT i = 0;i < g_uDispatchX * g_uDispatchY;++i)
	{
		fValue += pAddress[0];
	}
	fValue = fValue / g_uPixelCount;
	pResultBuffer->Unmap(0, nullptr);
	g_pHDRConstantBuffers->fValue = fValue;
*/
}

XRenderTarget* HDR_GetRenderTarget()
{
	return pRenderTarget;
}