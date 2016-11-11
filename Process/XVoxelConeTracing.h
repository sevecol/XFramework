
#pragma once

#include "..\XDirectX12.h"
#include "..\d3dx12.h"

bool InitVoxelConeTracing(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanVoxelConeTracing();

void VoxelConeTracing_Begin(ID3D12GraphicsCommandList *pCommandList,UINT uIndex);
void VoxelConeTracing_End(ID3D12GraphicsCommandList *pCommandList,UINT uIndex);
void VoxelConeTracing_Render(ID3D12GraphicsCommandList *pCommandList);