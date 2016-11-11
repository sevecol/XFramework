
#pragma once

#include "XResource.h"

#include "..\XDirectX12.h"
#include <map>

struct XGraphicShaderInfo
{
	LPCSTR pVSEntryPoint, pVSTarget;
	LPCSTR pPSEntryPoint, pPSTarget;
	LPCSTR pDSEntryPoint, pDSTarget;
	LPCSTR pHSEntryPoint, pHSTarget;
	LPCSTR pGSEntryPoint, pGSTarget;
};
struct XGraphicShaderInfo5 : public XGraphicShaderInfo
{
	XGraphicShaderInfo5(LPCSTR pVS, LPCSTR pPS, LPCSTR pDS, LPCSTR pHS, LPCSTR pGS)
	{
		pVSEntryPoint = pVS;
		pVSTarget = "vs_5_0";
		pPSEntryPoint = pPS;
		pPSTarget = "ps_5_0";
		pDSEntryPoint = pDS;
		pDSTarget = "ds_5_0";
		pHSEntryPoint = pHS;
		pHSTarget = "hs_5_0";
		pGSEntryPoint = pGS;
		pGSTarget = "gs_5_0";
	}
	XGraphicShaderInfo5(LPCSTR pVS, LPCSTR pPS)
	{
		pVSEntryPoint = pVS;
		pVSTarget = "vs_5_0";
		pPSEntryPoint = pPS;
		pPSTarget = "ps_5_0";

		pDSEntryPoint = nullptr;
		pHSEntryPoint = nullptr;
		pGSEntryPoint = nullptr;
	}
	XGraphicShaderInfo5(LPCSTR pVS, LPCSTR pPS, LPCSTR pGS)
	{
		pVSEntryPoint = pVS;
		pVSTarget = "vs_5_0";
		pPSEntryPoint = pPS;
		pPSTarget = "ps_5_0";
		pGSEntryPoint = pGS;
		pGSTarget = "gs_5_0";

		pDSEntryPoint = nullptr;
		pHSEntryPoint = nullptr;
	}
};

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
	static XGraphicShader* CreateGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_DEPTH_STENCIL_DESC &rDepthStencilDesc, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[]);
	static XGraphicShader* CreateGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount);
	static XGraphicShader* CreatePostProcessGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo);
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