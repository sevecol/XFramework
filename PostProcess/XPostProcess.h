
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

bool InitPostProcess(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanPostProcess();

void PostProcess_Bind(ID3D12GraphicsCommandList* pCommandList);