
#pragma once

#include "..\XDirectX12.h"

//
struct XShader
{
	ComPtr<ID3D12PipelineState>				m_pPipelineState;

public:
	virtual ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }
};

XShader* CreateShaderFromFile(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC *pInputElementDescs, UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);