#pragma once

#include "..\XDirectX12.h"
#include "..\Include\GFSDK_SSAO.h"

#define SSAO_NUM_DEPTH_SRV 1
#define SSAO_NUM_SRV (SSAO_NUM_DEPTH_SRV + GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12)
#define APP_NUM_RTV (7)
#define USE_BIN_MESH_READER 1

bool InitSSAO(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanSSAO();

void SSAO_Render(ID3D12GraphicsCommandList* pCommandList);