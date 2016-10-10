#include "XOrderIndependentTransparency.h"
#include "DXSampleHelper.h"

#include "Resource\XBuffer.h"
#include "Resource\XShader.h"

struct SAlphaInfo
{
	XMFLOAT4							m_fColor;
	UINT								m_uNext;
};
XStructuredBuffer<SAlphaInfo>			*g_pAlphaInfoBuffer;

struct SAlphaTag
{
	UINT								m_uFirst;
};
XStructuredBuffer<SAlphaTag>			*g_pAlphaTagBuffer;

extern ComPtr<ID3D12Device>				g_pDevice;
extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

XShader*								g_pOrderIndependentTransparencyShader;

//
bool InitOrderIndependentTransparency(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	g_pAlphaInfoBuffer	= new XStructuredBuffer<SAlphaInfo>(pDevice,uWidth*uHeight, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 7, g_uCSUDescriptorSize));
	g_pAlphaTagBuffer	= new XStructuredBuffer<SAlphaTag>(pDevice, uWidth*uHeight, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 8, g_uCSUDescriptorSize));
	
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
	SAFE_DELETE(g_pAlphaInfoBuffer);
	SAFE_DELETE(g_pAlphaTagBuffer);
	SAFE_DELETE(g_pOrderIndependentTransparencyShader);
}

void BeginOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize);
	
	//const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//pCommandList->ClearUnorderedAccessViewFloat(GpuHandle, CpuHandle, g_pAlphaInfoBuffer->GetBuffer(), clearColor, 0, nullptr);

	ID3D12DescriptorHeap *ppHeaps[] = { g_pCSUDescriptorHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->SetGraphicsRootDescriptorTable(3, CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 6, g_uCSUDescriptorSize));
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader);
void EndOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, g_pOrderIndependentTransparencyShader);
}