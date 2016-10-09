#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <vector>
#include <list>
#include <wrl.h>
using namespace Microsoft::WRL;

bool CreateDevice(HWND hWnd, UINT uWidth, UINT uHeight, bool bWindow);
void MoveToNextFrame();
void WaitForGpu();
bool Update();
bool Render();
void Clean();