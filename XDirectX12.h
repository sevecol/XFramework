
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
	ESHADINGPATH_POSTPROCESS,

	ESHADINGPATH_COUNT
};

#define	FRAME_NUM				3
#define RENDERTARGET_MAXNUM		3
#define LIGHT_MAXNUM			16

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

#define GRDT_CBV_FRAMEBUFFER		0			// 1,0,b0
#define GRDT_CBV_INSTANCEBUFFER		1			// 1,1,b1
#define GRDT_SRV_TEXTURE			2			// 3,0,t0-t2		// GBuffer,(albedo,normal,mask)...
#define GRDT_SRV_GLOBALTEXTURE		3			// 2,3,t3-t4		// EnvS,EnvD
#define GRDT_UVA_SBUFFER			4			// 3,0,u0-u2
#define GRDT_SRV_POSTPROCESSTEXTURE	5			// 1,5,t5			// PostProcess SrcTexture
#define GRDT_COUNT					6

#define CRDT_SRV_TEXTURE			0			// 3,0,t0-t2		// GBuffer,(albedo,normal,mask)...
#define CRDT_SRV_GLOBALTEXTURE		1			// 2,3,t3-t4		// EnvS,EnvD
#define CRDT_UVA_SRCSBUFFER			2			// 1,0,u0
#define CRDT_UVA_DSTSBUFFER			3			// 1,1,u1
#define CRDT_CBV_FRAMEBUFFER		4			// 1,0,b0
#define CRDT_CBV_INSTANCEBUFFER		5			// 1,1,b1
#define CRDT_COUNT					6

struct PointLight
{
	FLOAT			fPosX, fPosY, fPosZ, fAttenuationBegin;
	FLOAT			fR, fG, fB, fAttenuationEnd;
};
void AddPointLight(PointLight& sPointLight);
PointLight* GetPointLight(UINT uIndex);

//
class XEngine
{
public:
	// Windows
	HWND								m_hWnd;
	
	// Device
	ID3D12Device						*m_pDevice;
	ComPtr<IDXGISwapChain3>				m_pSwapChain;
	ComPtr<ID3D12CommandQueue>			m_pRenderCommandQueue;
	ComPtr<ID3D12RootSignature>			m_pGraphicRootSignature;
	ComPtr<ID3D12RootSignature>			m_pComputeRootSignature;

	HANDLE								m_hFenceEvent;
	ComPtr<ID3D12Fence>					m_pFence;

	// Heap|Resource
	struct XDescriptorHeap
	{
		ComPtr<ID3D12DescriptorHeap>	m_pDescriptorHeap;
		UINT							m_uSize;
		UINT							m_uStart;
		UINT							m_uCount;
	};

	enum XDescriptorHeapType
	{
		XDESCRIPTORHEAPTYPE_RTV = 0,
		XDESCRIPTORHEAPTYPE_DSV,
		XDESCRIPTORHEAPTYPE_GCSU,
		XDESCRIPTORHEAPTYPE_CCSU,

		XDESCRIPTORHEAPTYPE_COUNT
	};
	XDescriptorHeap						m_hHandleHeap[XDESCRIPTORHEAPTYPE_COUNT];
	ComPtr<ID3D12Resource>				m_pDepthStencil;

	//
	D3D12_VIEWPORT						m_Viewport;
	D3D12_RECT							m_ScissorRect;

	//
	//UIManager							g_UIManager;
};

