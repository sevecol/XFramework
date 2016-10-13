
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitHDR(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanHDR();

void HDR_Bind(ID3D12GraphicsCommandList* pCommandList);
void HDR_ToneMaping(ID3D12GraphicsCommandList* pCommandList);