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

#define SAFE_DELETE(p)              { if(p) { delete (p);       (p)=NULL; } }
#define SAFE_FREE(p)                { if(p) { free(p);          (p)=NULL; } }
#define SAFE_DELGRP(p)              { if(p) { delete[] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)             { if(p) { (p)->Release();   (p)=NULL; } }

enum ESHADINGPATH
{
	ESHADINGPATH_FORWORD	= 0,
	ESHADINGPATH_DEFERRED,

	ESHADINGPATH_COUNT
};

#define RENDERTARGET_MAXNUM		3