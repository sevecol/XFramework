
#include "XHDR.h"
#include "DXSampleHelper.h"

#include "Resource\XShader.h"
#include "Resource\XTexture.h"
#include "Resource\XBuffer.h"

// 9 RenderTarget,10 Texture,11,12 SBuffer,13 ConstantBuffer
#define GCSUBASE_HDR					9
#define CCSUBASE_HDR					3
#define RBASE_HDR						6

#define DISPATCHX_MAX					100
#define DISPATCHY_MAX					100
#define DISPATCHNUM_MAX					(DISPATCHX_MAX*DISPATCHY_MAX)
UINT									g_uDispatchX, g_uDispatchY, g_uPixelCount;

enum eHDRBuffer
{
	EHDRBUFFER_1 = 0,
	EHDRBUFFER_2,

	EHDRBUFFER_COUNT
};
IStructuredBuffer						*g_pHDRSBuffer[EHDRBUFFER_COUNT] = { nullptr,nullptr };
XRenderTarget							*g_pHDRRenderTarget			= nullptr;
XShader									*g_pHDRShaderToneMapping	= nullptr;
XComputeShader							*g_pHDRShaderLuminance		= nullptr;

XTextureSet								*g_pHDRTextureScreen		= nullptr;
XShader									*g_pHDRShaderScreen			= nullptr;

HDRConstantBuffer						*g_pHDRConstantBuffers		= nullptr;
ID3D12Resource							*g_pHDRConstantUploadHeap	= nullptr;

CD3DX12_CPU_DESCRIPTOR_HANDLE			g_hHDRUAVCpuHandle[EHDRBUFFER_COUNT];

extern XEngine							*g_pEngine;
extern ID3D12DescriptorHeap				*GetCpuCSUDHeap();
extern ID3D12DescriptorHeap				*GetGpuCSUDHeap();
extern UINT								GetCSUDHeapSize();
extern XResourceThread					*g_pResourceThread;

//
ID3D12Resource *pResultBuffer = nullptr;
bool InitHDR(ID3D12Device* pDevice,UINT uWidth, UINT uHeight)
{
	//
	g_uDispatchX = min(uWidth / 16, DISPATCHX_MAX);
	g_uDispatchY = min(uHeight / 16, DISPATCHY_MAX);
	g_uPixelCount = uWidth * uHeight;

	// ResultBuffer
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(DISPATCHNUM_MAX * sizeof(float), D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&pResultBuffer)));

	// ConstantBuffer
	{
		// Create an upload heap for the constant buffers.
		// HDRConstant
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(HDRConstantBuffer)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_pHDRConstantUploadHeap)));

		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(g_pHDRConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&g_pHDRConstantBuffers)));
		g_pHDRConstantBuffers->uDispatchX = g_uDispatchX;
		g_pHDRConstantBuffers->uDispatchY = g_uDispatchY;

		//
		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
		ConstantDesc.BufferLocation = g_pHDRConstantUploadHeap->GetGPUVirtualAddress();
		ConstantDesc.SizeInBytes = sizeof(HDRConstantBuffer);
		pDevice->CreateConstantBufferView(&ConstantDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetCPUDescriptorHandleForHeapStart(), GCSUBASE_HDR + 4, GetCSUDHeapSize()));
	}

	// StructuredBuffer
	for (UINT i = 0;i < EHDRBUFFER_COUNT;++i)
	{
		g_pHDRSBuffer[i] = new XStructuredBuffer<float>(pDevice, 100*100, CD3DX12_CPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetCPUDescriptorHandleForHeapStart(), GCSUBASE_HDR+2 +i, GetCSUDHeapSize()));
		g_pHDRSBuffer[i]->SetUAVGpuHandle(CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetGPUDescriptorHandleForHeapStart(), GCSUBASE_HDR+2 +i, GetCSUDHeapSize()));
		
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = DISPATCHNUM_MAX;
		UDesc.Buffer.StructureByteStride = sizeof(float);
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		g_hHDRUAVCpuHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCpuCSUDHeap()->GetCPUDescriptorHandleForHeapStart(), CCSUBASE_HDR + i, GetCSUDHeapSize());
		pDevice->CreateUnorderedAccessView(g_pHDRSBuffer[i]->GetResource(), nullptr, &UDesc, g_hHDRUAVCpuHandle[i]);
	}

	// RenderTarget
	g_pHDRRenderTarget = XRenderTarget::CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, uWidth, uHeight, RBASE_HDR, GCSUBASE_HDR);

	// Shader
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

	// Texture
	LPCWSTR lpTextureFileName[] = {L"hdr.dds"};
	g_pHDRTextureScreen = XTextureSet::CreateTextureSet(L"HDRTexture", 1, lpTextureFileName, GCSUBASE_HDR+1);

	return true;
}

void CleanHDR()
{
	// ConstantBuffer
	if (g_pHDRConstantUploadHeap)
	{
		g_pHDRConstantUploadHeap->Unmap(0, nullptr);
		SAFE_RELEASE(g_pHDRConstantUploadHeap);
	}
	g_pHDRConstantBuffers = nullptr;

	// StructuredBuffer
	SAFE_RELEASE(pResultBuffer);
	for (UINT i = 0;i < EHDRBUFFER_COUNT;++i)
	{
		SAFE_DELETE(g_pHDRSBuffer[i]);
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
void HDR_Luminance(ID3D12GraphicsCommandList* pCommandList);
void HDR_ToneMapping(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pCommandList->SetGraphicsRootDescriptorTable(2, g_pHDRRenderTarget->GetSRVGpuHandle());

	//
	HDR_Luminance(pCommandList);

	//
	pCommandList->SetGraphicsRootDescriptorTable(1, CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetGPUDescriptorHandleForHeapStart(), GCSUBASE_HDR + 4, GetCSUDHeapSize()));

	// ToneMapping
	RenderFullScreen(pCommandList, g_pHDRShaderToneMapping);
}

void HDR_Luminance(ID3D12GraphicsCommandList* pCommandList)
{
	// ComputeShader
	FLOAT fClearValue[] = { 0.0f,0.0f, };
	pCommandList->ClearUnorderedAccessViewFloat(g_pHDRSBuffer[0]->GetUAVGpuHandle(), g_hHDRUAVCpuHandle[0], g_pHDRSBuffer[0]->GetResource(), &fClearValue[0], 0, nullptr);
	pCommandList->ClearUnorderedAccessViewFloat(g_pHDRSBuffer[1]->GetUAVGpuHandle(), g_hHDRUAVCpuHandle[1], g_pHDRSBuffer[1]->GetResource(), &fClearValue[1], 0, nullptr);

	//
	pCommandList->SetComputeRootSignature(g_pEngine->m_pComputeRootSignature.Get());
	pCommandList->SetPipelineState(g_pHDRShaderLuminance->GetPipelineState());

	pCommandList->SetComputeRootDescriptorTable(0, g_pHDRRenderTarget->GetSRVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(1, g_pHDRSBuffer[0]->GetUAVGpuHandle());
	pCommandList->SetComputeRootDescriptorTable(2, CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuCSUDHeap()->GetGPUDescriptorHandleForHeapStart(), GCSUBASE_HDR + 4, GetCSUDHeapSize()));

	//
	pCommandList->Dispatch(g_uDispatchX, g_uDispatchY, 1);

	// GetResult
	int iIndex = 0;
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRSBuffer[iIndex]->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	pCommandList->CopyResource(pResultBuffer, g_pHDRSBuffer[iIndex]->GetResource());
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_pHDRSBuffer[iIndex]->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	float *pAddress = nullptr;
	CD3DX12_RANGE readRange(0, g_uDispatchX * g_uDispatchY * sizeof(float));
	pResultBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pAddress));
	//float fValue = *pAddress;

	float fValue = 0.0f;
	for (UINT i = 0;i < g_uDispatchX * g_uDispatchY;++i)
	{
		fValue += pAddress[i];
	}
	fValue = fValue / g_uPixelCount;
	pResultBuffer->Unmap(0, nullptr);
	g_pHDRConstantBuffers->fValue = fValue;
}