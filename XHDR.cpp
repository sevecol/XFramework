#include "XHDR.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"
#include "Resource\XBuffer.h"

extern XEngine							*g_pEngine;
extern XResourceThread					*g_pResourceThread;

IStructuredBuffer						*g_pHDRStructuredBuffer[2];
XRenderTarget							*g_pHDRRenderTarget			= nullptr;
XShader									*g_pHDRShaderToneMapping	= nullptr;
XComputeShader							*g_pHDRShaderLuminance		= nullptr;

XTextureSet								*g_pHDRTextureScreen		= nullptr;
XShader									*g_pHDRShaderScreen			= nullptr;

CD3DX12_CPU_DESCRIPTOR_HANDLE			g_hHDRUAVCpuHandle[2];

//
bool InitHDR(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	g_pHDRStructuredBuffer[0] = new XStructuredBuffer<float>(pDevice, 10000, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 11, g_pEngine->m_uCSUDescriptorSize));
	g_pHDRStructuredBuffer[0]->SetUAVGpuHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 11, g_pEngine->m_uCSUDescriptorSize));
	g_pHDRStructuredBuffer[1] = new XStructuredBuffer<float>(pDevice, 10000, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 12, g_pEngine->m_uCSUDescriptorSize));
	g_pHDRStructuredBuffer[1]->SetUAVGpuHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pEngine->m_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 11, g_pEngine->m_uCSUDescriptorSize));

	for (UINT i = 0;i < 2;++i)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = 10000;
		UDesc.Buffer.StructureByteStride = sizeof(float);
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		g_hHDRUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pEngine->m_pUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 3+i, g_pEngine->m_uCSUDescriptorSize);
		pDevice->CreateUnorderedAccessView(g_pHDRStructuredBuffer[i]->GetResource(), nullptr, &UDesc, g_hHDRUAVCpuHandle[i]);
	}

	//
	g_pHDRRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, uWidth, uHeight, 6, 9);

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	DXGI_FORMAT Format[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	g_pHDRShaderToneMapping = XShader::CreateShaderFromFile(L"shaders_hdr_tonemapping.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3,1, Format);
	g_pHDRShaderLuminance = XComputeShader::CreateComputeShaderFromFile(L"shaders_hdr_luminance.hlsl", "CSMain", "cs_5_0");

	//
	Format[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	g_pHDRShaderScreen = XShader::CreateShaderFromFile(L"shaders_hdr_screen.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3, 1, Format);

	//
	LPCWSTR lpTextureFileName[] = {L"hdr.dds"};
	g_pHDRTextureScreen = XTextureSet::CreateTextureSet(L"HDRTexture", 1, lpTextureFileName, 10);

	return true;
}

void CleanHDR()
{
	// StructuredBuffer
	for (UINT i = 0;i < 2;++i)
	{
		SAFE_DELETE(g_pHDRStructuredBuffer[i]);
	}

	// RenderTarget
	SAFE_DELETE(g_pHDRRenderTarget);

	// Shader
	SAFE_DELETE(g_pHDRShaderToneMapping);
	SAFE_DELETE(g_pHDRShaderLuminance);
	SAFE_DELETE(g_pHDRShaderScreen);

	// Texture
	XTextureSet::DeleteTextureSet(&g_pHDRTextureScreen);
}

void HDR_Bind(ID3D12GraphicsCommandList *pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DHandle(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	pCommandList->OMSetRenderTargets(1, &g_pHDRRenderTarget->GetRTVCpuHandle(), FALSE, &DHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pCommandList->ClearRenderTargetView(g_pHDRRenderTarget->GetRTVCpuHandle(), clearColor, 0, nullptr);
	pCommandList->ClearDepthStencilView(g_pEngine->m_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XShader *pShader, XTextureSet *pTexture = nullptr);
void HDR_ToneMapping(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pHDRRenderTarget->GetSRVGpuHandle());//CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 9, g_uCSUDescriptorSize));

	// ComputeShader
	FLOAT fClearValue[] = { 0.0f,0.0f, };
	pCommandList->ClearUnorderedAccessViewFloat(g_pHDRStructuredBuffer[0]->GetUAVGpuHandle(), g_hHDRUAVCpuHandle[0], g_pHDRStructuredBuffer[0]->GetResource(), &fClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewFloat(g_pHDRStructuredBuffer[1]->GetUAVGpuHandle(), g_hHDRUAVCpuHandle[1], g_pHDRStructuredBuffer[1]->GetResource(), &fClearValue[1], 0, nullptr);

	//
	pCommandList->SetPipelineState(g_pHDRShaderLuminance->GetPipelineState());
	pCommandList->SetComputeRootSignature(g_pEngine->m_pComputeRootSignature.Get());

	pCommandList->SetComputeRootDescriptorTable(0, g_pHDRTextureScreen->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(1, g_pHDRStructuredBuffer[0]->GetUAVGpuHandle());

	//
	pCommandList->Dispatch(1280/16, 720/16, 1);

	// GetResult?

	// ToneMapping
	RenderFullScreen(pCommandList, g_pHDRShaderToneMapping);
}