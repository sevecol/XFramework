
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitDeferredShading(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanDeferredShading();

void DeferredShading_GBuffer(ID3D12GraphicsCommandList* pCommandList);
void DeferredShading_Shading(ID3D12GraphicsCommandList* pCommandList);