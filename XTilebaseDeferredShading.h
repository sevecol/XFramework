
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitDeferredShading(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanDeferredShading();
void BeginDeferredShading(ID3D12GraphicsCommandList* pCommandList);
void EndDeferredShading(ID3D12GraphicsCommandList* pCommandList);