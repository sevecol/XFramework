#include "XTilebaseDeferredShading.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "UI\UIManager.h"

#define DEFERREDSHADING_RENDERTARGET_RBASE		3
#define DEFERREDSHADING_CONSTANT_CSUBASE		3
#define DEFERREDSHADING_RENDERTARGET_COUNT		RENDERTARGET_MAXNUM

extern UINT								g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT						g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];

extern ComPtr<ID3D12DescriptorHeap>		g_pRDescriptorHeap;
extern ComPtr<ID3D12DescriptorHeap>		g_pDDescriptorHeap;
extern UINT								g_uRDescriptorSize;

extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

extern UIManager						g_UIManager;

ID3D12Resource*							g_pDRRenderTargets[DEFERREDSHADING_RENDERTARGET_COUNT] = { nullptr,nullptr,nullptr };
XShader*								g_pDeferredShadingShader;

//
bool InitDeferredShading(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	for (unsigned int i = 0;i < DEFERREDSHADING_RENDERTARGET_COUNT;++i)
	{
		D3D12_RESOURCE_DESC textureDesc = {};

		textureDesc.MipLevels = 1;
		textureDesc.Format = g_RenderTargetFortmat[ESHADINGPATH_DEFERRED][i];
		textureDesc.Width = uWidth;
		textureDesc.Height = uHeight;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		D3D12_CLEAR_VALUE rendertargetOptimizedClearValue = {};
		rendertargetOptimizedClearValue.Format = textureDesc.Format;
		rendertargetOptimizedClearValue.Color[0] = 0.0f;
		rendertargetOptimizedClearValue.Color[1] = 0.0f;
		rendertargetOptimizedClearValue.Color[2] = 0.0f;
		rendertargetOptimizedClearValue.Color[3] = 0.0f;

		//
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&rendertargetOptimizedClearValue,
			IID_PPV_ARGS(&g_pDRRenderTargets[i])));
		pDevice->CreateRenderTargetView(g_pDRRenderTargets[i], nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), DEFERREDSHADING_RENDERTARGET_RBASE +i, g_uRDescriptorSize));
		
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		pDevice->CreateShaderResourceView(g_pDRRenderTargets[i], &srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), DEFERREDSHADING_CONSTANT_CSUBASE +i, g_uCSUDescriptorSize));
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	g_pDeferredShadingShader = CreateShaderFromFile(L"shaders_deferredshading.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, ESHADINGPATH_DEFERRED);

	return true;
}

void CleanDeferredShading()
{
	SAFE_DELETE(g_pDeferredShadingShader);
	for (unsigned int i = 0;i < 3;++i)
	{
		SAFE_RELEASE(g_pDRRenderTargets[i]);
	}
}

void BeginDeferredShading(ID3D12GraphicsCommandList* pCommandList)
{
	// Indicate that the back buffer will be used as a render target.
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE RHandle[3];
	RHandle[0] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), DEFERREDSHADING_RENDERTARGET_RBASE+0, g_uRDescriptorSize);
	RHandle[1] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), DEFERREDSHADING_RENDERTARGET_RBASE+1, g_uRDescriptorSize);
	RHandle[2] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), DEFERREDSHADING_RENDERTARGET_RBASE+2, g_uRDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(3, RHandle, FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pCommandList->ClearRenderTargetView(RHandle[0], clearColor, 0, nullptr);
	//pCommandList->ClearRenderTargetView(RHandle[1], clearColor, 0, nullptr);
	//pCommandList->ClearRenderTargetView(RHandle[2], clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(g_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
/*
	//
	RenderFullScreen(pCommandList,g_pDeferredShadingShader);
*/
}

void EndDeferredShading(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[2], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	ID3D12DescriptorHeap *ppHeaps[] = { g_pCSUDescriptorHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->SetGraphicsRootDescriptorTable(2, CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), DEFERREDSHADING_RENDERTARGET_RBASE, g_uCSUDescriptorSize));
}