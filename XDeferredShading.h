
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"
#include "Resource\XTexture.h"

bool InitDeferredShading(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanDeferredShading();

void DeferredShading_GBuffer(ID3D12GraphicsCommandList* pCommandList);
void DeferredShading_Shading(ID3D12GraphicsCommandList* pCommandList);
void XM_CALLCONV DeferredShading_Update(FXMMATRIX view, CXMMATRIX projection);

struct LightConstantBuffer
{
	XMFLOAT4X4		mView;						// View Rotation matrix.
	XMFLOAT4X4		mProj;						// Projection (P) matrix.
	XMFLOAT4X4		mViewProj;					// View Projection (VP) matrix.
	PointLight		sLight[LIGHT_MAXNUM];
	XMFLOAT4		vEyePos;
	XMFLOAT4		vCameraNF;
	UINT			uLightNum;
	FLOAT			fPadding[7];
};