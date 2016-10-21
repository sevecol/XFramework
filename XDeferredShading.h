
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitDeferredShading(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanDeferredShading();

void DeferredShading_GBuffer(ID3D12GraphicsCommandList* pCommandList);
void DeferredShading_Shading(ID3D12GraphicsCommandList* pCommandList);
void XM_CALLCONV DeferredShading_Update(FXMMATRIX view, CXMMATRIX projection);

struct LightConstantBuffer
{
	XMFLOAT4X4		mMvp;						// Model-view-projection (MVP) matrix.
	XMFLOAT4X4		mP;							// Projection (P) matrix.
	PointLight		sLight[LIGHT_MAXNUM];
	UINT			uLightNum;
	FLOAT			fPadding[31];
};