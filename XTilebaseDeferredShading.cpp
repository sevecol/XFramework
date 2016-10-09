#include "XTilebaseDeferredShading.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "UI\UIManager.h"

extern ComPtr<ID3D12DescriptorHeap>		g_pRtvHeap;
extern ComPtr<ID3D12DescriptorHeap>		g_pDsvHeap;
extern UINT								g_uRtvDescriptorSize;

extern ComPtr<ID3D12DescriptorHeap>		g_pCbvSrvUavHeap;
extern UINT								g_uCSUDescriptorSize;

extern UIManager						g_UIManager;

ID3D12Resource*							g_pDRRenderTargets[3] = { nullptr,nullptr,nullptr };
XShader*								g_pDeferredShadingShader;

bool InitDeferredShading(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	D3D12_RESOURCE_DESC textureDesc[3];

	for (unsigned int i = 0;i < 3;++i)
	{
		textureDesc[i] = D3D12_RESOURCE_DESC();

		textureDesc[i].MipLevels = 1;
		//textureDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc[i].Width = uWidth;
		textureDesc[i].Height = uHeight;
		textureDesc[i].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		textureDesc[i].DepthOrArraySize = 1;
		textureDesc[i].SampleDesc.Count = 1;
		textureDesc[i].SampleDesc.Quality = 0;
		textureDesc[i].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	}
	textureDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc[2].Format = DXGI_FORMAT_R16G16_FLOAT;

	for (unsigned int i = 0;i < 3;++i)
	{
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc[i],
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&g_pDRRenderTargets[i])));
		pDevice->CreateRenderTargetView(g_pDRRenderTargets[i], nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3+i, g_uRtvDescriptorSize));
		
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc[i].Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		pDevice->CreateShaderResourceView(g_pDRRenderTargets[i], &srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), i, g_uCSUDescriptorSize));
	}

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	DXGI_FORMAT RenderTargetFormat[8];
	RenderTargetFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	RenderTargetFormat[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	RenderTargetFormat[2] = DXGI_FORMAT_R16G16_FLOAT;
	g_pDeferredShadingShader = CreateShaderFromFile(L"shaders_deferredshading.hlsl", "VSMain", "vs_5_0", "PSMainForDeferredShading", "ps_5_0", inputElementDescs, 3,3, RenderTargetFormat);

	return true;
}

void CleanDeferredShading()
{
	if (g_pDeferredShadingShader)
	{
		delete g_pDeferredShadingShader;
	}
	for (unsigned int i = 0;i < 3;++i)
	{
		if (g_pDRRenderTargets[i])
		{
			g_pDRRenderTargets[i]->Release();
		}
	}
}

void BeginDeferredShading(ID3D12GraphicsCommandList* pCommandList)
{
	// Indicate that the back buffer will be used as a render target.
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[2], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[3];
	rtvHandle[0] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, g_uRtvDescriptorSize);
	rtvHandle[1] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, g_uRtvDescriptorSize);
	rtvHandle[2] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, g_uRtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(g_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(3, rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 1.2f, 0.4f, 1.0f };
	//pCommandList->ClearRenderTargetView(rtvHandle[0], clearColor, 0, nullptr);
	//pCommandList->ClearRenderTargetView(rtvHandle[1], clearColor, 0, nullptr);
	//pCommandList->ClearRenderTargetView(rtvHandle[2], clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(g_pDsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//
	pCommandList->SetPipelineState(g_pDeferredShadingShader->GetPipelineState());
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap *ppHeaps[] = { g_pCbvSrvUavHeap.Get() };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

	//
	XGeometry* pGeometry = g_UIManager.GetGeometry();
	pCommandList->IASetVertexBuffers(0, 1, pGeometry->GetVertexBufferView());
	if (pGeometry->GetNumIndices())
	{
		pCommandList->IASetIndexBuffer(pGeometry->GetIndexBufferView());
		pCommandList->DrawIndexedInstanced(pGeometry->GetNumIndices(), 1, 0, 0, 0);
	}
}

void EndDeferredShading(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pDRRenderTargets[2], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}