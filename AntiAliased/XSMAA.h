
#pragma once

#include "..\XDirectX12.h"

bool InitSMAA(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanSMAA();

void SMAA_Render(ID3D12GraphicsCommandList* pCommandList);