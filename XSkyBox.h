
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitSkyBox(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanSkyBox();

void SkyBox_Render(ID3D12GraphicsCommandList* pCommandList);