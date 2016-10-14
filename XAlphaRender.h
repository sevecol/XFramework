
#pragma once

#include "XDirectX12.h"
#include "d3dx12.h"

bool InitAlphaRender(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
void CleanAlphaRender();

void AlphaRender_PreRender(ID3D12GraphicsCommandList* pCommandList);
void AlphaRender_Begin(ID3D12GraphicsCommandList* pCommandList);
void AlphaRender_End(ID3D12GraphicsCommandList* pCommandList);