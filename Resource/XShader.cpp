
#include "d3dCompiler.h"
#include "XShader.h"

#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

extern XEngine *g_pEngine;

extern UINT	g_uRenderTargetCount[ESHADINGPATH_COUNT];
extern DXGI_FORMAT g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM];

//
std::map<std::wstring, XShader*> XShader::m_mShader;
XShader* XShader::CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath)
{
	return CreateShaderFromFile(pFileName, pVSEntryPoint, pVSTarget, pPSEntryPoint, pPSTarget, pInputElementDescs, uInputElementCount, g_uRenderTargetCount[eShadingPath], g_RenderTargetFortmat[eShadingPath]);
}

//
XShader* XShader::CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[])
{
	XShader *pShader = nullptr;
	std::map<std::wstring, XShader*>::iterator it = XShader::m_mShader.find(pFileName);
	if (it != XShader::m_mShader.end())
	{
		pShader = it->second;
		if (pShader)
		{
			pShader->AddRef();
		}
		return pShader;
	}

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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc = {};
	gpsoDesc.InputLayout = { pInputElementDescs, uInputElementCount };
	gpsoDesc.pRootSignature = g_pEngine->m_pGraphicRootSignature.Get();
	gpsoDesc.RasterizerState = rasterizerStateDesc;
	gpsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsoDesc.DepthStencilState = depthStencilDesc;
	gpsoDesc.SampleMask = UINT_MAX;
	gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsoDesc.NumRenderTargets = uRenderTargetCount;
	for (unsigned int i = 0;i < gpsoDesc.NumRenderTargets;++i)
	{
		gpsoDesc.RTVFormats[i] = RenderTargetFormat[i];
	}
	gpsoDesc.SampleDesc.Count = 1;
	gpsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pVSEntryPoint, pVSTarget, compileFlags, 0, &vertexShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pPSEntryPoint, pPSTarget, compileFlags, 0, &pixelShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}
	
	gpsoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	gpsoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };

	pShader = new XShader(pFileName);
	XShader::m_mShader[pFileName] = pShader;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateGraphicsPipelineState(&gpsoDesc, IID_PPV_ARGS(&pShader->m_pPipelineState)));
	
	return pShader;
}

//
XShader* XShader::CreateShaderFromFile(LPCWSTR pFileName, D3D12_DEPTH_STENCIL_DESC &depthstencildesc, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath)
{
	XShader *pShader = nullptr;
	std::map<std::wstring, XShader*>::iterator it = XShader::m_mShader.find(pFileName);
	if (it != XShader::m_mShader.end())
	{
		pShader = it->second;
		if (pShader)
		{
			pShader->AddRef();
		}
		return pShader;
	}

	//
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#ifdef _DEBUG
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = 0;//D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
	rasterizerStateDesc.CullMode = D3D12_CULL_MODE_NONE;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc = {};
	gpsoDesc.InputLayout = { pInputElementDescs, uInputElementCount };
	gpsoDesc.pRootSignature = g_pEngine->m_pGraphicRootSignature.Get();
	gpsoDesc.RasterizerState = rasterizerStateDesc;
	gpsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsoDesc.DepthStencilState = depthstencildesc;
	gpsoDesc.SampleMask = UINT_MAX;
	gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsoDesc.NumRenderTargets = g_uRenderTargetCount[eShadingPath];
	for (unsigned int i = 0;i < gpsoDesc.NumRenderTargets;++i)
	{
		gpsoDesc.RTVFormats[i] = g_RenderTargetFortmat[eShadingPath][i];
	}
	gpsoDesc.SampleDesc.Count = 1;
	gpsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pVSEntryPoint, pVSTarget, compileFlags, 0, &vertexShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pPSEntryPoint, pPSTarget, compileFlags, 0, &pixelShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}

	gpsoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	gpsoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };

	pShader = new XShader(pFileName);
	XShader::m_mShader[pFileName] = pShader;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateGraphicsPipelineState(&gpsoDesc, IID_PPV_ARGS(&pShader->m_pPipelineState)));

	return pShader;
}

void XShader::DeleteShader(XShader** ppShader)
{
	if (*ppShader)
	{
		int iRef = (*ppShader)->DecRef();
		if (iRef <= 0)
		{
			std::map<std::wstring, XShader*>::iterator it = XShader::m_mShader.find((*ppShader)->GetName());
			if (it != XShader::m_mShader.end())
			{
				XShader::m_mShader.erase(it);
			}
			SAFE_DELETE(*ppShader);
		}
		*ppShader = nullptr;
	}
}

XComputeShader* XComputeShader::CreateComputeShaderFromFile(LPCWSTR pFileName, LPCSTR pCSEntryPoint, LPCSTR pCSTarget)
{
	//
	ComPtr<ID3DBlob> computeShader;

#ifdef _DEBUG
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	//
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pCSEntryPoint, pCSTarget, compileFlags, 0, &computeShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
	}

	// Describe and create the compute pipeline state object (PSO).
	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsoDesc = {};
	cpsoDesc.pRootSignature = g_pEngine->m_pComputeRootSignature.Get();
	cpsoDesc.CS = { reinterpret_cast<UINT8*>(computeShader->GetBufferPointer()), computeShader->GetBufferSize() };
	cpsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	XComputeShader* pShader = new XComputeShader;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateComputePipelineState(&cpsoDesc, IID_PPV_ARGS(&pShader->m_pPipelineState)));

	return pShader;
}