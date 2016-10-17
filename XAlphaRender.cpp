
#include "XAlphaRender.h"
#include "DXSampleHelper.h"

#include "Resource\XBuffer.h"
#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#define MAX_PIXELS	32
// 6,7,8 SBuffer
#define GCSUBASE_ALPHARENDER			6
#define CCSUBASE_ALPHARENDER			0

enum eAlphaRenderBuffer
{
	EALPHARENDERBUFFER_COUNTER			= 0,
	EALPHARENDERBUFFER_PIXELLINK,
	EALPHARENDERBUFFER_STARTOFFSET,

	EALPHARENDERBUFFER_COUNT
};
IStructuredBuffer						*g_pOITSBuffer[EALPHARENDERBUFFER_COUNT];
CD3DX12_CPU_DESCRIPTOR_HANDLE			g_hOITUAVCpuHandle[EALPHARENDERBUFFER_COUNT];

struct SPixelLink
{
	XMFLOAT4							m_fColor;
	FLOAT								m_fDepth;
	UINT								m_uNext;
};

XShader*								g_pAlphaRenderShader;

extern XEngine							*g_pEngine;
extern ID3D12DescriptorHeap				*GetCpuCSUDHeap();
extern ID3D12DescriptorHeap				*GetGpuCSUDHeap();
extern UINT								GetCSUDHeapSize();

//
bool InitAlphaRender(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hUAVCpuHandle[EALPHARENDERBUFFER_COUNT];
	CD3DX12_GPU_DESCRIPTOR_HANDLE hUAVGpuHandle[EALPHARENDERBUFFER_COUNT];

	for (UINT i = 0;i < EALPHARENDERBUFFER_COUNT;++i)
	{
		hUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetCPUDescriptorHandleForHeapStart(), GCSUBASE_ALPHARENDER + i, GetCSUDHeapSize());
		hUAVGpuHandle[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetGPUDescriptorHandleForHeapStart(), GCSUBASE_ALPHARENDER + i, GetCSUDHeapSize());
	}

	g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER] = new XStructuredBuffer<UINT>(pDevice, 1, hUAVCpuHandle[EALPHARENDERBUFFER_COUNTER]);
	g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->SetUAVGpuHandle(hUAVGpuHandle[EALPHARENDERBUFFER_COUNTER]);

	g_pOITSBuffer[EALPHARENDERBUFFER_PIXELLINK] = new XStructuredBuffer<SPixelLink>(pDevice, uWidth*uHeight*MAX_PIXELS, hUAVCpuHandle[EALPHARENDERBUFFER_PIXELLINK], true, g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->GetResource());
	g_pOITSBuffer[EALPHARENDERBUFFER_PIXELLINK]->SetUAVGpuHandle(hUAVGpuHandle[EALPHARENDERBUFFER_PIXELLINK]);

	g_pOITSBuffer[EALPHARENDERBUFFER_STARTOFFSET] = new XStructuredBuffer<UINT>(pDevice, uWidth*uHeight, hUAVCpuHandle[EALPHARENDERBUFFER_STARTOFFSET]);
	g_pOITSBuffer[EALPHARENDERBUFFER_STARTOFFSET]->SetUAVGpuHandle(hUAVGpuHandle[EALPHARENDERBUFFER_STARTOFFSET]);

	//
	UINT uSize[EALPHARENDERBUFFER_COUNT] = { sizeof(UINT),sizeof(SPixelLink),sizeof(UINT) };
	UINT uNum[EALPHARENDERBUFFER_COUNT] = { 1,uWidth*uHeight*MAX_PIXELS ,uWidth*uHeight };
	for (UINT i = 0;i < EALPHARENDERBUFFER_COUNT;++i)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = uNum[i];
		UDesc.Buffer.StructureByteStride = uSize[i];
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		g_hOITUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCpuCSUDHeap()->GetCPUDescriptorHandleForHeapStart(), CCSUBASE_ALPHARENDER+i, GetCSUDHeapSize());
		pDevice->CreateUnorderedAccessView(g_pOITSBuffer[i]->GetResource(), nullptr, &UDesc, g_hOITUAVCpuHandle[i]);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	g_pAlphaRenderShader = XShader::CreateShaderFromFile(L"shaders_oit_final.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_FORWORD);

	return true;
}

void CleanAlphaRender()
{
	for (UINT i = 0;i < EALPHARENDERBUFFER_COUNT;++i)
	{
		SAFE_DELETE(g_pOITSBuffer[i]);
	}
	SAFE_DELETE(g_pAlphaRenderShader);
}

void AlphaRender_PreRender(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->SetGraphicsRootDescriptorTable(3, g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->GetUAVGpuHandle());
}

void AlphaRender_Begin(ID3D12GraphicsCommandList* pCommandList)
{
	UINT uClearValue[] = { 0,0xFFFFFFFF, };
	pCommandList->ClearUnorderedAccessViewUint(g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->GetUAVGpuHandle(), g_hOITUAVCpuHandle[EALPHARENDERBUFFER_COUNTER], g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->GetResource(), &uClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewUint(g_pOITSBuffer[EALPHARENDERBUFFER_STARTOFFSET]->GetUAVGpuHandle(), g_hOITUAVCpuHandle[EALPHARENDERBUFFER_STARTOFFSET], g_pOITSBuffer[EALPHARENDERBUFFER_STARTOFFSET]->GetResource(), &uClearValue[1], 0, nullptr);

	pCommandList->SetGraphicsRootDescriptorTable(3, g_pOITSBuffer[EALPHARENDERBUFFER_COUNTER]->GetUAVGpuHandle());
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void AlphaRender_End(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, g_pAlphaRenderShader);
}