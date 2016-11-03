
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

#include "Resource\XBuffer.h"

bool InitHDR(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanHDR();

void HDR_Bind(ID3D12GraphicsCommandList* pCommandList);
void HDR_ToneMapping(ID3D12GraphicsCommandList* pCommandList);
class XRenderTarget;
XRenderTarget* HDR_GetRenderTarget();

IStructuredBuffer* GetHDRSBuffer(UINT uIndex);
ID3D12Resource* GetHDRResultBuffer();

struct HDRConstantBuffer
{
	UINT			uDispatchX, uDispatchY;
	UINT			uScreenX, uScreenY;
	FLOAT			fValue;
	FLOAT			fPadding[59];
};