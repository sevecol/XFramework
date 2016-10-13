
#include "d3dCompiler.h"
#include "XShader.h"

#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

extern XEngine							*g_pEngine;

extern UINT								g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT						g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];

//
XShader* XShader::CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath)
{
	return CreateShaderFromFile(pFileName, pVSEntryPoint, pVSTarget, pPSEntryPoint, pPSTarget, pInputElementDescs, uInputElementCount, g_uRenderTargetCount[eShadingPath], g_RenderTargetFortmat[eShadingPath]);
}

//
XShader* XShader::CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[])
{
	//
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#ifdef _DEBUG
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = 0;//D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
	rasterizerStateDesc.CullMode = D3D12_CULL_MODE_NONE;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { pInputElementDescs, uInputElementCount };
	psoDesc.pRootSignature = g_pEngine->m_pRootSignature.Get();
	psoDesc.RasterizerState = rasterizerStateDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = uRenderTargetCount;
	for (unsigned int i = 0;i < psoDesc.NumRenderTargets;++i)
	{
		psoDesc.RTVFormats[i] = RenderTargetFormat[i];
	}
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, nullptr, pVSEntryPoint, pVSTarget, compileFlags, 0, &vertexShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, nullptr, pPSEntryPoint, pPSTarget, compileFlags, 0, &pixelShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}
	

	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };

	XShader* pShader = new XShader;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pShader->m_pPipelineState)));
	
	return pShader;
}