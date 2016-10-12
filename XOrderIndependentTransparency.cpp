#include "XOrderIndependentTransparency.h"
#include "DXSampleHelper.h"

#include "Resource\XBuffer.h"
#include "Resource\XShader.h"

#define MAX_PIXELS	32

XStructuredBuffer<UINT>					*g_pCounterBuffer;
struct SPixelLink
{
	XMFLOAT4							m_fColor;
	FLOAT								m_fDepth;
	UINT								m_uNext;
};
XStructuredBuffer<SPixelLink>			*g_pPixelLinkBuffer;
XStructuredBuffer<UINT>					*g_pStartOffsetBuffer;

extern ComPtr<ID3D12Device>				g_pDevice;
extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

XShader*								g_pOrderIndependentTransparencyShader;

//
ComPtr<ID3D12DescriptorHeap>			g_pCpuDescriptorHeap;
bool InitOrderIndependentTransparency(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	g_pCounterBuffer		= new XStructuredBuffer<UINT>(pDevice, 1, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize));
	g_pPixelLinkBuffer		= new XStructuredBuffer<SPixelLink>(pDevice,uWidth*uHeight*MAX_PIXELS, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 7, g_uCSUDescriptorSize),true, g_pCounterBuffer->GetBuffer());
	g_pStartOffsetBuffer	= new XStructuredBuffer<UINT>(pDevice, uWidth*uHeight, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 8, g_uCSUDescriptorSize));
	
	//
	// Describe and create a constant buffer view (CBV), Shader resource
	// view (SRV), and unordered access view (UAV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC CSUHeapDesc = {};
	CSUHeapDesc.NumDescriptors = 2;
	CSUHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CSUHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&CSUHeapDesc, IID_PPV_ARGS(&g_pCpuDescriptorHeap)));

	//
	D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
	UDesc.Format = DXGI_FORMAT_UNKNOWN;
	UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UDesc.Buffer.FirstElement = 0;
	UDesc.Buffer.NumElements = 1;
	UDesc.Buffer.StructureByteStride = sizeof(UINT);
	UDesc.Buffer.CounterOffsetInBytes = 0;
	UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	pDevice->CreateUnorderedAccessView(g_pCounterBuffer->GetBuffer(), nullptr, &UDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, g_uCSUDescriptorSize));

	//
	UDesc.Buffer.NumElements = uWidth*uHeight;
	pDevice->CreateUnorderedAccessView(g_pStartOffsetBuffer->GetBuffer(), nullptr, &UDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, g_uCSUDescriptorSize));

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	g_pOrderIndependentTransparencyShader = CreateShaderFromFile(L"shaders_oitfinal.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_DEFERRED);

	return true;
}

void CleanOrderIndependentTransparency()
{
	SAFE_DELETE(g_pCounterBuffer);
	SAFE_DELETE(g_pPixelLinkBuffer);
	SAFE_DELETE(g_pStartOffsetBuffer);
	SAFE_DELETE(g_pOrderIndependentTransparencyShader);
}

void BeginOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle0 = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, g_uCSUDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, g_uCSUDescriptorSize);
	
	UINT uClearValue[] = { 0xFFFFFFFF,0 };
	pCommandList->ClearUnorderedAccessViewUint(CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 8, g_uCSUDescriptorSize), CpuHandle1, g_pStartOffsetBuffer->GetBuffer(), &uClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewUint(CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize), CpuHandle0, g_pCounterBuffer->GetBuffer(), &uClearValue[1], 0, nullptr);

	ID3D12DescriptorHeap *ppHeaps[] = { g_pCSUDescriptorHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->SetGraphicsRootDescriptorTable(3, CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize));
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader);
void EndOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, g_pOrderIndependentTransparencyShader);
}