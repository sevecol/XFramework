
#include "XSkyBox.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#define GCSUBASE_SKYBOX					16

XShader									*g_pSkyBoxShader	= nullptr;
XTextureSet								*g_pSkyBoxTexture	= nullptr;

bool InitSkyBox(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
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
	g_pSkyBoxShader = XShader::CreateShaderFromFile(L"shaders_skybox_ds.hlsl", depthStencilDesc, "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);

	// Texture
	g_pSkyBoxTexture = XTextureSet::CreateCubeTexture(L"SkyBoxTexture", L"skybox.dds", GCSUBASE_SKYBOX);

	return true;
}
void CleanSkyBox()
{
	SAFE_DELETE(g_pSkyBoxShader);
	SAFE_DELETE(g_pSkyBoxTexture);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void SkyBox_Render(ID3D12GraphicsCommandList* pCommandList)
{
	RenderFullScreen(pCommandList, g_pSkyBoxShader, g_pSkyBoxTexture);
}