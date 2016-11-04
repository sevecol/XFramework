
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

bool InitScreenSpaceReflection(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanScreenSpaceReflection();

void ScreenSpaceReflection_Render(ID3D12GraphicsCommandList* pCommandList);