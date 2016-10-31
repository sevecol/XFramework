
#pragma once

#include "XResource.h"

#include "..\XDirectX12.h"
#include <map>

//
struct XShader : public XResource
{
	ComPtr<ID3D12PipelineState>						m_pPipelineState;
	static std::map<std::wstring, XShader*>			m_mShader;

public:
	XShader(LPCWSTR pName) :XResource(pName){}

	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }

	static XShader* CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[]);
	static XShader* CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
	static XShader* CreateShaderFromFile(LPCWSTR pFileName, D3D12_DEPTH_STENCIL_DESC &depthstencildesc, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
	static void DeleteShader(XShader** ppShader);
};

struct XComputeShader
{
	ComPtr<ID3D12PipelineState>				m_pPipelineState;
public:
	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }

	static XComputeShader* CreateComputeShaderFromFile(LPCWSTR pFileName, LPCSTR pCSEntryPoint, LPCSTR pCSTarget);
};

