
#include "d3dCompiler.h"
#include "XShader.h"
#include "..\SceneGraph\XNode.h"

#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

extern XEngine *g_pEngine;

D3D12_INPUT_ELEMENT_DESC FullScreenElementDescs[] =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};
UINT uFullScreenElementCount = 3;

D3D12_INPUT_ELEMENT_DESC StandardElementDescs[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};
UINT uStandardElementCount = 4;

//
std::map<std::wstring, XGraphicShader*> XGraphicShaderManager::m_mResource;
XGraphicShader* XGraphicShaderManager::CreateGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount)
{
	//
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	//
	DXGI_FORMAT RenderTargetFortmat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT };
	return CreateGraphicShaderFromFile(pFileName, rGraphicShaderInfo, depthStencilDesc, pInputElementDescs, uInputElementCount, 1, RenderTargetFortmat);
}

XGraphicShader* XGraphicShaderManager::CreatePostProcessGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo)
{
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	//
	DXGI_FORMAT RenderTargetFortmat[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	return CreateGraphicShaderFromFile(pFileName, rGraphicShaderInfo, depthStencilDesc, FullScreenElementDescs, uFullScreenElementCount, 1, RenderTargetFortmat);
}

//
D3D12_PRIMITIVE_TOPOLOGY_TYPE gTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
XGraphicShader* XGraphicShaderManager::CreateGraphicShaderFromFile(LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_DEPTH_STENCIL_DESC &rDepthStencilDesc, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[])
{
	//
	XGraphicShader *pGraphicShader = GetResource(pFileName);
	if (pGraphicShader)
	{
		return pGraphicShader;
	}

	//
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	ComPtr<ID3DBlob> domainShader;
	ComPtr<ID3DBlob> hullShader;
	ComPtr<ID3DBlob> geometryShader;

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
	gpsoDesc.DepthStencilState = rDepthStencilDesc;
	gpsoDesc.SampleMask = UINT_MAX;
	gpsoDesc.PrimitiveTopologyType = gTopologyType;
	gpsoDesc.NumRenderTargets = uRenderTargetCount;
	for (unsigned int i = 0;i < gpsoDesc.NumRenderTargets;++i)
	{
		gpsoDesc.RTVFormats[i] = RenderTargetFormat[i];
	}
	gpsoDesc.SampleDesc.Count = 1;
	gpsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//
	if (rGraphicShaderInfo.pVSEntryPoint)
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, rGraphicShaderInfo.pVSEntryPoint, rGraphicShaderInfo.pVSTarget, compileFlags, 0, &vertexShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		gpsoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	}
	if (rGraphicShaderInfo.pPSEntryPoint)
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, rGraphicShaderInfo.pPSEntryPoint, rGraphicShaderInfo.pPSTarget, compileFlags, 0, &pixelShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		gpsoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	}
	if (rGraphicShaderInfo.pDSEntryPoint)
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, rGraphicShaderInfo.pDSEntryPoint, rGraphicShaderInfo.pDSTarget, compileFlags, 0, &domainShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		gpsoDesc.DS = { reinterpret_cast<UINT8*>(domainShader->GetBufferPointer()), domainShader->GetBufferSize() };
	}
	if (rGraphicShaderInfo.pHSEntryPoint)
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, rGraphicShaderInfo.pHSEntryPoint, rGraphicShaderInfo.pHSTarget, compileFlags, 0, &hullShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		gpsoDesc.HS = { reinterpret_cast<UINT8*>(hullShader->GetBufferPointer()), hullShader->GetBufferSize() };
	}
	if (rGraphicShaderInfo.pGSEntryPoint)
	{
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3DCompileFromFile(pFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, rGraphicShaderInfo.pGSEntryPoint, rGraphicShaderInfo.pGSTarget, compileFlags, 0, &geometryShader, &pError);
		if (hr != S_OK)
		{
			OutputDebugStringA((char*)(pError->GetBufferPointer()));
		}
		ThrowIfFailed(hr);
		gpsoDesc.GS = { reinterpret_cast<UINT8*>(geometryShader->GetBufferPointer()), geometryShader->GetBufferSize() };
	}
	
	//
	pGraphicShader = new XGraphicShader(pFileName);
	AddResource(pFileName, pGraphicShader);
	ThrowIfFailed(g_pEngine->m_pDevice->CreateGraphicsPipelineState(&gpsoDesc, IID_PPV_ARGS(&pGraphicShader->m_pPipelineState)));
	
	return pGraphicShader;
}

//
std::map<std::wstring, XComputeShader*> XComputeShaderManager::m_mResource;
XComputeShader* XComputeShaderManager::CreateComputeShaderFromFile(LPCWSTR pFileName, LPCSTR pCSEntryPoint, LPCSTR pCSTarget)
{
	XComputeShader *pComputeShader = GetResource(pFileName);
	if (pComputeShader)
	{
		return pComputeShader;
	}

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

	pComputeShader = new XComputeShader(pFileName);
	AddResource(pFileName, pComputeShader);
	ThrowIfFailed(g_pEngine->m_pDevice->CreateComputePipelineState(&cpsoDesc, IID_PPV_ARGS(&pComputeShader->m_pPipelineState)));

	return pComputeShader;
}