
#include "XAlphaRender.h"
#include "DXSampleHelper.h"

#include "Resource\XBuffer.h"
#include "Resource\XShader.h"
#include "Resource\XTexture.h"

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

	XShader*								pShader;
}
using namespace AlphaRender;

//
bool InitAlphaRender(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,3);
	uCpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_CCSU,3);

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

		hUAVCpuHandle[i] = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_CCSU, uCpuCSUBase+i);
		pDevice->CreateUnorderedAccessView(pSBuffer[i]->GetResource(), nullptr, &UDesc, hUAVCpuHandle[i]);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShader = XShader::CreateShaderFromFile(L"shaders_oit_final.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_FORWORD);

	return true;
}

void CleanAlphaRender()
{
	for (UINT i = 0;i < ESBUFFERTYPE_COUNT;++i)
	{
		SAFE_DELETE(pSBuffer[i]);
	}
	SAFE_DELETE(pShader);
}

void AlphaRender_PreRender(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->SetGraphicsRootDescriptorTable(3, pSBuffer[ESBUFFERTYPE_COUNTER]->GetUAVGpuHandle());
}

void AlphaRender_Begin(ID3D12GraphicsCommandList* pCommandList)
{
	UINT uClearValue[] = { 0,0xFFFFFFFF, };
	pCommandList->ClearUnorderedAccessViewUint(pSBuffer[ESBUFFERTYPE_COUNTER]->GetUAVGpuHandle(), hUAVCpuHandle[ESBUFFERTYPE_COUNTER], pSBuffer[ESBUFFERTYPE_COUNTER]->GetResource(), &uClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewUint(pSBuffer[ESBUFFERTYPE_STARTOFFSET]->GetUAVGpuHandle(), hUAVCpuHandle[ESBUFFERTYPE_STARTOFFSET], pSBuffer[ESBUFFERTYPE_STARTOFFSET]->GetResource(), &uClearValue[1], 0, nullptr);

	pCommandList->SetGraphicsRootDescriptorTable(3, pSBuffer[ESBUFFERTYPE_COUNTER]->GetUAVGpuHandle());
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void AlphaRender_End(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, pShader);
}