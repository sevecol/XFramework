
#include "XSkyBox.h"

#include "..\Resource\XShader.h"
#include "..\Resource\XTexture.h"

extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

namespace SkyBox
{
	UINT								uGpuCSUBase;

	XGraphicShader						*pSkyBoxShader		= nullptr;
	XTextureSet							*pSkyBoxTexture		= nullptr;
	XTextureSet							*pSkyDiffuseTexture = nullptr;
}
using namespace SkyBox;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool InitSkyBox(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	//
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,2);

	// Texture
	pSkyBoxTexture = XTextureSetManager::CreateCubeTexture(L"SkyBoxTexture", L"skybox.dds", uGpuCSUBase);
	pSkyDiffuseTexture = XTextureSetManager::CreateCubeTexture(L"SkyDiffuseTexture", L"skydiffuse.dds", uGpuCSUBase+1);

	// Shader
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	pSkyBoxShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"shaders_skybox_ds.hlsl", depthStencilDesc, "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);

	return true;
}
void CleanSkyBox()
{
	XGraphicShaderManager::DelResource(&pSkyBoxShader);
	XTextureSetManager::DelResource(&pSkyBoxTexture);
	XTextureSetManager::DelResource(&pSkyDiffuseTexture);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void SkyBox_Render(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, pSkyBoxShader, pSkyBoxTexture);
}

XTextureSet* GetSkyBoxTexture()
{
	return pSkyBoxTexture;
}