
#include "XScreenSpaceReflection.h"

#include "Resource\XGeometry.h"
#include "Resource\XShader.h"
#include "Resource\XTexture.h"

#include "XHDR.h"

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
/*
	IStructuredBuffer* pSBuffer = GetHDRSBuffer(0);
	ID3D12Resource* pResultBuffer = GetHDRResultBuffer();
	pCommandList->SetGraphicsRootDescriptorTable(4, pSBuffer->GetUAVGpuHandle());
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