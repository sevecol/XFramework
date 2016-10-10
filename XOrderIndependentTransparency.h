
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitOrderIndependentTransparency(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanOrderIndependentTransparency();
void BeginOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList);
void EndOrderIndependentTransparency(ID3D12GraphicsCommandList* pCommandList);