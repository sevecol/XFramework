
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
	XMFLOAT4X4		mViewR;						// View Rotation matrix.
	XMFLOAT4X4		mProj;						// Projection (P) matrix.
	XMFLOAT4X4		mViewProj;					// View Projection (VP) matrix.
	PointLight		sLight[LIGHT_MAXNUM];
	UINT			uLightNum;
	FLOAT			fPadding[15];
};