
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

class XFrameResource
{
public:
	//
	UINT									m_uIndex;

	struct ConstantBuffer
	{
		XMFLOAT4X4 mMv;
		XMFLOAT4X4 mMvp;					// Model-view-projection (MVP) matrix.
		XMFLOAT4X4 mMvpInv;					// Model-view-projection (MVP) matrix.
		FLOAT fPadding[16];
	};
	ConstantBuffer*							m_pConstantBuffers;
	//XMFLOAT4X4							m_modelMatrices;

	ComPtr<ID3D12CommandAllocator>			m_pRenderCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>		m_pCommandList;
	ComPtr<ID3D12Resource>					m_pRenderTargets;
	ComPtr<ID3D12Resource>					m_pConstantUploadHeap;
	//ComPtr<ID3D12PipelineState>			m_pPipelineState;
	UINT64									m_uFenceValue;

public:
	~XFrameResource();

	static void Init(ID3D12Device* pDevice);
	void InitInstance(UINT uIndex,ID3D12Device* pDevice, IDXGISwapChain3 *pSwapChain);
	void PreRender();
	void BeginRender();
	void EndRender();

	void XM_CALLCONV UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX projection);
};