
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitOrderIndependentTransparency(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanOrderIndependentTransparency();

void OrderIndependentTransparency_Begin(ID3D12GraphicsCommandList* pCommandList);
void OrderIndependentTransparency_End(ID3D12GraphicsCommandList* pCommandList);