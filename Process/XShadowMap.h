
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

bool InitShadowMap(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanShadowMap();

void ShadowMap_Begin(ID3D12GraphicsCommandList *pCommandList);
void ShadowMap_End(ID3D12GraphicsCommandList *pCommandList);
void XM_CALLCONV ShadowMap_Update(FXMMATRIX view, CXMMATRIX projection);