
#pragma once

#include "XResource.h"

#include "..\XDirectX12.h"
#include <map>

//
struct XGraphicShader : public XResource
{
	ComPtr<ID3D12PipelineState>						m_pPipelineState;

public:
	XGraphicShader(LPCWSTR pName) :XResource(pName){}

	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }
};
class XGraphicShaderManager : public XResourceManager<XGraphicShader>
{
public:
	static XGraphicShader* CreateGraphicShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[]);
	static XGraphicShader* CreateGraphicShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
	static XGraphicShader* CreateGraphicShaderFromFile(LPCWSTR pFileName, D3D12_DEPTH_STENCIL_DESC &depthstencildesc, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
};

struct XComputeShader : public XResource
{
	ComPtr<ID3D12PipelineState>						m_pPipelineState;

public:
	XComputeShader(LPCWSTR pName) :XResource(pName) {}

	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }
};
class XComputeShaderManager : public XResourceManager<XComputeShader>
{
public:
	static XComputeShader* CreateComputeShaderFromFile(LPCWSTR pFileName, LPCSTR pCSEntryPoint, LPCSTR pCSTarget);
};

