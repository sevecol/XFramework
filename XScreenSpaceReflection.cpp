
#include "XScreenSpaceReflection.h"

#include "Resource\XGeometry.h"
#include "Resource\XShader.h"
#include "Resource\XTexture.h"

namespace ScreenSpaceReflection
{
	UINT										uGpuCSUBase;
	XGraphicShader								*pShadingShader = nullptr;
}
using namespace ScreenSpaceReflection;

bool InitScreenSpaceReflection(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	pShadingShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_screenspacereflection.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4);

	return true;
}
void CleanScreenSpaceReflection()
{
	XGraphicShaderManager::DelResource(&pShadingShader);
}

extern void RenderXZPlane(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void ScreenSpaceReflection_Render(ID3D12GraphicsCommandList* pCommandList)
{
	RenderXZPlane(pCommandList, pShadingShader);
}