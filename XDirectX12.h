
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

#include "Thread\XResourceThread.h"
#include "Resource\XFrameResource.h"
//#include "UI\UIManager.h"

enum ESHADINGPATH
{
	ESHADINGPATH_FORWORD = 0,
	ESHADINGPATH_DEFERRED,

	ESHADINGPATH_COUNT
};

#define	FRAME_NUM				3
#define RENDERTARGET_MAXNUM		3

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

class XEngine
{
public:
	// Windows
	HWND								m_hWnd;
	
	// Device
	ID3D12Device						*m_pDevice;
	ComPtr<IDXGISwapChain3>				m_pSwapChain;
	ComPtr<ID3D12CommandQueue>			m_pRenderCommandQueue;
	ComPtr<ID3D12RootSignature>			m_pRootSignature;

	HANDLE								m_hFenceEvent;
	ComPtr<ID3D12Fence>					m_pFence;

	// Heap|Resource
	ComPtr<ID3D12DescriptorHeap>		m_pRDescriptorHeap;
	UINT								m_uRDescriptorSize;

	ComPtr<ID3D12DescriptorHeap>		m_pDDescriptorHeap;
	ComPtr<ID3D12Resource>				m_pDepthStencil;

	ComPtr<ID3D12DescriptorHeap>		m_pCSUDescriptorHeap;
	UINT								m_uCSUDescriptorSize;

	//
	D3D12_VIEWPORT						m_Viewport;
	D3D12_RECT							m_ScissorRect;

	//
	//UIManager							g_UIManager;
};

