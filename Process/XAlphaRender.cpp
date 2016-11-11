
#include "XAlphaRender.h"
#include "..\DXSampleHelper.h"

#include "..\Resource\XBuffer.h"
#include "..\Resource\XShader.h"
#include "..\Resource\XTexture.h"

#define MAX_PIXELS	32

extern XEngine *g_pEngine;
extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

namespace AlphaRender
{
	UINT									uGpuCSUBase;
	UINT									uCpuCSUBase;
	UINT									uDispatchX, uDispatchY;

	enum eSBufferType
	{
		ESBUFFERTYPE_COUNTER = 0,
		ESBUFFERTYPE_PIXELLINK,
		ESBUFFERTYPE_STARTOFFSET,

		ESBUFFERTYPE_COUNT
	};
	IStructuredBuffer						*pSBuffer[ESBUFFERTYPE_COUNT];
	CD3DX12_CPU_DESCRIPTOR_HANDLE			hUAVCpuHandle[ESBUFFERTYPE_COUNT];

	struct SPixelLink
	{
		XMFLOAT4							m_fColor;
		FLOAT								m_fDepth;
		UINT								m_uNext;
	};

	XGraphicShader							*pGraphicShader;
	XComputeShader							*pComputeShader;

	//
	ID3D12Resource							*pResultBuffer = nullptr;
}
using namespace AlphaRender;

//
bool InitAlphaRender(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,3);
	uCpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_CCSU,3);

	//
	uDispatchX = uWidth / 32 + 1;
	uDispatchY = uHeight / 32 + 1;

	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hUAVCpuHandle[ESBUFFERTYPE_COUNT];
	CD3DX12_GPU_DESCRIPTOR_HANDLE hUAVGpuHandle[ESBUFFERTYPE_COUNT];

	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		hUAVCpuHandle[i] = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + i);
		hUAVGpuHandle[i] = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + i);
	}

	pSBuffer[ESBUFFERTYPE_COUNTER] = new XStructuredBuffer<UINT>(pDevice, 1, hUAVCpuHandle[ESBUFFERTYPE_COUNTER]);
	pSBuffer[ESBUFFERTYPE_COUNTER]->SetUAVGpuHandle(hUAVGpuHandle[ESBUFFERTYPE_COUNTER]);

	pSBuffer[ESBUFFERTYPE_PIXELLINK] = new XStructuredBuffer<SPixelLink>(pDevice, uWidth*uHeight*MAX_PIXELS, hUAVCpuHandle[ESBUFFERTYPE_PIXELLINK], true, pSBuffer[ESBUFFERTYPE_COUNTER]->GetResource());
	pSBuffer[ESBUFFERTYPE_PIXELLINK]->SetUAVGpuHandle(hUAVGpuHandle[ESBUFFERTYPE_PIXELLINK]);

	pSBuffer[ESBUFFERTYPE_STARTOFFSET] = new XStructuredBuffer<UINT>(pDevice, uWidth*uHeight, hUAVCpuHandle[ESBUFFERTYPE_STARTOFFSET]);
	pSBuffer[ESBUFFERTYPE_STARTOFFSET]->SetUAVGpuHandle(hUAVGpuHandle[ESBUFFERTYPE_STARTOFFSET]);

	//
	UINT uSize[ESBUFFERTYPE_COUNT] = { sizeof(UINT),sizeof(SPixelLink),sizeof(UINT) };
	UINT uNum[ESBUFFERTYPE_COUNT] = { 1,uWidth*uHeight*MAX_PIXELS ,uWidth*uHeight };
	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = uNum[i];
		UDesc.Buffer.StructureByteStride = uSize[i];
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		AlphaRender::hUAVCpuHandle[i] = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_CCSU, uCpuCSUBase+i);
		pDevice->CreateUnorderedAccessView(pSBuffer[i]->GetResource(), nullptr, &UDesc, AlphaRender::hUAVCpuHandle[i]);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pGraphicShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_oit_final.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_FORWORD);
	pComputeShader = XComputeShaderManager::CreateComputeShaderFromFile(L"Media\\shaders_oit_finalcs.hlsl", "CSMain", "cs_5_0");

	// ResultBuffer
/*
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

void CleanAlphaRender()
{
	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		SAFE_DELETE(pSBuffer[i]);
	}
	XGraphicShaderManager::DelResource(&pGraphicShader);
	XComputeShaderManager::DelResource(&pComputeShader);

	//
	SAFE_RELEASE(pResultBuffer);
}

void AlphaRender_Begin(ID3D12GraphicsCommandList* pCommandList)
{
	UINT uClearValue[] = { 0,0xFFFFFFFF, };
	pCommandList->ClearUnorderedAccessViewUint(pSBuffer[ESBUFFERTYPE_COUNTER]->GetUAVGpuHandle(), hUAVCpuHandle[ESBUFFERTYPE_COUNTER], pSBuffer[ESBUFFERTYPE_COUNTER]->GetResource(), &uClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewUint(pSBuffer[ESBUFFERTYPE_STARTOFFSET]->GetUAVGpuHandle(), hUAVCpuHandle[ESBUFFERTYPE_STARTOFFSET], pSBuffer[ESBUFFERTYPE_STARTOFFSET]->GetResource(), &uClearValue[1], 0, nullptr);

	pCommandList->SetGraphicsRootDescriptorTable(GRDT_UVA_SBUFFER, pSBuffer[ESBUFFERTYPE_COUNTER]->GetUAVGpuHandle());
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
extern XRenderTarget* HDR_GetRenderTarget();
void AlphaRender_End(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->OMSetRenderTargets(0, nullptr, true, nullptr);

	//
	XRenderTarget* pHDRRenderTarget = HDR_GetRenderTarget();
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	//
	pCommandList->SetPipelineState(pComputeShader->GetPipelineState());

	//pCommandList->SetComputeRootDescriptorTable(0, );
	pCommandList->SetComputeRootDescriptorTable(CRDT_UVA_SRCSBUFFER, pHDRRenderTarget->GetUAVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(CRDT_UVA_DSTSBUFFER, pSBuffer[ESBUFFERTYPE_PIXELLINK]->GetUAVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(CRDT_CBV_INSTANCEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase + 3));

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

	//RenderFullScreen(pCommandList, pShader);
}

IStructuredBuffer* GetAlphaRenderSBuffer()
{
	return pSBuffer[0];
}