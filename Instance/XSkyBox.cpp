
#include "XSkyBox.h"

#include "..\Resource\XShader.h"
#include "..\Resource\XTexture.h"

extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

extern D3D12_INPUT_ELEMENT_DESC FullScreenElementDescs[];
extern UINT uFullScreenElementCount;

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
	pSkyBoxTexture = XTextureSetManager::CreateCubeTexture(L"SkyBoxTexture", L"Media\\skybox.dds", uGpuCSUBase);
	pSkyDiffuseTexture = XTextureSetManager::CreateCubeTexture(L"SkyDiffuseTexture", L"Media\\skydiffuse.dds", uGpuCSUBase+1);

	// Shader
	pSkyBoxShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_skybox_ds.hlsl", XGraphicShaderInfo5("VSMain", "PSMain"), FullScreenElementDescs, uFullScreenElementCount);

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