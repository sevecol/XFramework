#include "XOrderIndependentTransparency.h"
#include "DXSampleHelper.h"

#include "Resource\XBuffer.h"
#include "Resource\XShader.h"

#define MAX_PIXELS	32

enum eOrderIndependentTransparencyBuffer
{
	EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER		= 0,
	EORDERINDEPENDENTTRANSPARENCYBUFFER_PIXELLINK,
	EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET,

	EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT
};
IStructuredBuffer						*p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT];
CD3DX12_CPU_DESCRIPTOR_HANDLE			g_hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT];

struct SPixelLink
{
	XMFLOAT4							m_fColor;
	FLOAT								m_fDepth;
	UINT								m_uNext;
};

extern XEngine							*g_pEngine;
XShader*								g_pOrderIndependentTransparencyShader;

//
ComPtr<ID3D12DescriptorHeap>			g_pCpuDescriptorHeap;
bool InitOrderIndependentTransparency(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT];
	CD3DX12_GPU_DESCRIPTOR_HANDLE hUAVGpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT];

	for (UINT i = 0;i < EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT;++i)
	{
		hUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 6 + i, g_pEngine->m_uCSUDescriptorSize);
		hUAVGpuHandle[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 6 + i, g_pEngine->m_uCSUDescriptorSize);
	}

	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER] = new XStructuredBuffer<UINT>(pDevice, 1, hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]);
	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]->SetUAVGpuHandle(hUAVGpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]);

	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_PIXELLINK] = new XStructuredBuffer<SPixelLink>(pDevice, uWidth*uHeight*MAX_PIXELS, hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_PIXELLINK], true, p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]->GetResource());
	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_PIXELLINK]->SetUAVGpuHandle(hUAVGpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_PIXELLINK]);

	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET] = new XStructuredBuffer<UINT>(pDevice, uWidth*uHeight, hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET]);
	p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET]->SetUAVGpuHandle(hUAVGpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET]);

	//
	// Describe and create a constant buffer view (CBV), Shader resource
	// view (SRV), and unordered access view (UAV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC CSUHeapDesc = {};
	CSUHeapDesc.NumDescriptors = 3;
	CSUHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CSUHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&CSUHeapDesc, IID_PPV_ARGS(&g_pCpuDescriptorHeap)));

	UINT uSize[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT] = { sizeof(UINT),sizeof(SPixelLink),sizeof(UINT) };
	UINT uNum[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT] = { 1,uWidth*uHeight*MAX_PIXELS ,uWidth*uHeight };
	for (UINT i = 0;i < EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT;++i)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = uNum[i];
		UDesc.Buffer.StructureByteStride = uSize[i];
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		g_hUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, g_pEngine->m_uCSUDescriptorSize);
		pDevice->CreateUnorderedAccessView(p_pStructuredBuffer[i]->GetResource(), nullptr, &UDesc, g_hUAVCpuHandle[i]);
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	g_pOrderIndependentTransparencyShader = XShader::CreateShaderFromFile(L"shaders_oit_final.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_FORWORD);

	return true;
}

void CleanOrderIndependentTransparency()
{
	for (UINT i = 0;i < EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNT;++i)
	{
		SAFE_DELETE(p_pStructuredBuffer[i]);
	}
	SAFE_DELETE(g_pOrderIndependentTransparencyShader);
}

void OrderIndependentTransparency_Begin(ID3D12GraphicsCommandList* pCommandList)
{
	UINT uClearValue[] = { 0,0xFFFFFFFF, };
	pCommandList->ClearUnorderedAccessViewUint(p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]->GetUAVGpuHandle(), g_hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER], p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]->GetResource(), &uClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewUint(p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET]->GetUAVGpuHandle(), g_hUAVCpuHandle[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET], p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_STARTOFFSET]->GetResource(), &uClearValue[1], 0, nullptr);

	pCommandList->SetGraphicsRootDescriptorTable(3, p_pStructuredBuffer[EORDERINDEPENDENTTRANSPARENCYBUFFER_COUNTER]->GetUAVGpuHandle());
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader);
void OrderIndependentTransparency_End(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, g_pOrderIndependentTransparencyShader);
}