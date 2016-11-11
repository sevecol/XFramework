
#include "XScreenSpaceReflection.h"

#include "..\..\Resource\XGeometry.h"
#include "..\..\Resource\XShader.h"
#include "..\..\Resource\XTexture.h"
#include "..\..\XHDR.h"

extern D3D12_INPUT_ELEMENT_DESC StandardElementDescs[];
extern UINT uStandardElementCount;

namespace ScreenSpaceReflection
{
	XGraphicShader								*pShadingShader = nullptr;
}
using namespace ScreenSpaceReflection;

bool InitScreenSpaceReflection(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	// Shader
	pShadingShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\shaders_screenspacereflection.hlsl", XGraphicShaderInfo5("VSMain", "PSMain"), StandardElementDescs, uStandardElementCount);

	return true;
}
void CleanScreenSpaceReflection()
{
	XGraphicShaderManager::DelResource(&pShadingShader);
}

extern void RenderXZPlane(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void ScreenSpaceReflection_Render(ID3D12GraphicsCommandList* pCommandList)
{
/*
	IStructuredBuffer* pSBuffer = GetHDRSBuffer(0);
	ID3D12Resource* pResultBuffer = GetHDRResultBuffer();
	pCommandList->SetGraphicsRootDescriptorTable(GRDT_UVA_SBUFFER, pSBuffer->GetUAVGpuHandle());
*/
	RenderXZPlane(pCommandList, pShadingShader);
/*
	// GetResult
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetSBuffer(1)->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	pCommandList->CopyResource(pResultBuffer, GetSBuffer(1)->GetResource());
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetSBuffer(1)->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	float *pAddress = nullptr;
	CD3DX12_RANGE readRange(0, 4 * sizeof(float));
	pResultBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pAddress));
	//float fValue = *pAddress;

	float fValue = 0.0f;
	//for (UINT i = 0;i < g_uDispatchX * g_uDispatchY;++i)
	{
		fValue += pAddress[0];
	}
	pResultBuffer->Unmap(0, nullptr);
	//g_pHDRConstantBuffers->fValue = fValue;
*/
}