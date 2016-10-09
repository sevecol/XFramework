
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

class XFrameResource
{
public:
	//
	UINT									m_uIndex;

	ComPtr<ID3D12CommandAllocator>			m_pRenderCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>		m_pCommandList;
	ComPtr<ID3D12Resource>					m_pRenderTargets;
	ComPtr<ID3D12Resource>					m_pCbvUploadHeap;
	ComPtr<ID3D12PipelineState>				m_pPipelineState;
	UINT64									m_uFenceValue;

public:
	~XFrameResource();

	void InitInstance(UINT uIndex,ID3D12Device* pDevice, IDXGISwapChain3 *pSwapChain);
	void PreRender();
	void BeginRender();
	void EndRender();
};