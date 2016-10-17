
#pragma once

#include "..\XDirectX12.h"

//
struct XShader
{
	ComPtr<ID3D12PipelineState>				m_pPipelineState;
public:
	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }

	static XShader* CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[]);
	static XShader* CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);

	static XShader* CreateShaderFromFile(LPCWSTR pFileName, D3D12_DEPTH_STENCIL_DESC &depthstencildesc, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
};

struct XComputeShader
{
	ComPtr<ID3D12PipelineState>				m_pPipelineState;
public:
	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }

	static XComputeShader* CreateComputeShaderFromFile(LPCWSTR pFileName, LPCSTR pCSEntryPoint, LPCSTR pCSTarget);
};

