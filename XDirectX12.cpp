
#include "XDirectX12.h"

#include "XDeferredShading.h"
#include "XHDR.h"

#include "Process\XShadowMap.h"
#include "Process\XAlphaRender.h"
#include "Process\XVoxelConeTracing.h"
#include "Process\PostProcess\XPostProcess.h"
#include "Process\PostProcess\XScreenSpaceReflection.h"
#include "Process\PostProcess\XSMAA.h"
#include "Process\PostProcess\XSSAO.h"

#include "SceneGraph\XSceneGraph.h"

#include "Resource\XBuffer.h"
#include "Resource\XTexture.h"

#include "Instance\XEntity.h"
#include "Instance\XSkyBox.h"
#include "Instance\XCamera.h"

#include "StepTimer.h"

#include <d2d1_3.h>
#include <dwrite.h>

#include <D3Dcompiler.h>
#include "d3dx12.h"
#include <DirectXMath.h>
#include <d3d11on12.h>
#include "DXSampleHelper.h"

//
XEngine *g_pEngine = nullptr;
ID3D12DescriptorHeap *GetHandleHeap(XEngine::XDescriptorHeapType eType)
{
	return g_pEngine->m_hHandleHeap[eType].m_pDescriptorHeap.Get();
}
D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(GetHandleHeap(eType)->GetCPUDescriptorHandleForHeapStart(), uIndex, g_pEngine->m_hHandleHeap[eType].m_uSize);
}
D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(GetHandleHeap(eType)->GetGPUDescriptorHandleForHeapStart(), uIndex, g_pEngine->m_hHandleHeap[eType].m_uSize);
}
UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType,UINT uCount)
{
	if ((g_pEngine->m_hHandleHeap[eType].m_uStart + uCount) > g_pEngine->m_hHandleHeap[eType].m_uCount)
	{
		return 0xFFFFFFFF;
	}

	UINT uStart = g_pEngine->m_hHandleHeap[eType].m_uStart;
	g_pEngine->m_hHandleHeap[eType].m_uStart += uCount;

	return uStart;
}

void CreateDefaultDepthBuffer(ID3D12Resource** ppResource,UINT uWidth,UINT uHeight, D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor)
{
	// Create the depth stencil.
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, uWidth, uHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(ppResource)
		));

	g_pEngine->m_pDevice->CreateDepthStencilView(*ppResource, &depthStencilDesc, hDescriptor);
}

// FrameResource
UINT								g_uFrameIndex;
XFrameResource						*g_pFrameResource[FRAME_NUM];
XResourceThread						*g_pResourceThread;

XCamera								g_Camera;
StepTimer							g_Timer;

//
XSceneGraph							g_SceneGraph;

//
extern XGraphicShader				*g_pHDRShaderScreen;
extern XTextureSet					*g_pHDRTextureScreen;

//
bool CreateDevice(HWND hWnd, UINT uWidth, UINT uHeight, bool bWindow)
{
	g_pEngine = new XEngine;
	g_pEngine->m_hWnd = hWnd;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CreateDevice
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#ifdef _DEBUG
	// Enable the D3D12 debug layer.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
	/*
	if (m_useWarpDevice)
	{
	ComPtr<IDXGIAdapter> warpAdapter;
	ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

	ThrowIfFailed(D3D12CreateDevice(
	warpAdapter.Get(),
	D3D_FEATURE_LEVEL_11_0,
	IID_PPV_ARGS(&m_device)
	));
	}
	else
	*/
	{
		ThrowIfFailed(D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&g_pEngine->m_pDevice)
			));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Describe and create the command queues.
	D3D12_COMMAND_QUEUE_DESC renderqueueDesc = {};
	renderqueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	renderqueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_pEngine->m_pDevice->CreateCommandQueue(&renderqueueDesc, IID_PPV_ARGS(&g_pEngine->m_pRenderCommandQueue)));

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_NUM;//FrameCount;
	swapChainDesc.BufferDesc.Width = uWidth;
	swapChainDesc.BufferDesc.Height = uHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = bWindow;

	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(factory->CreateSwapChain(
		g_pEngine->m_pRenderCommandQueue.Get(),		// Swap chain needs the render queue so that it can force a flush on it.
		&swapChainDesc,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&g_pEngine->m_pSwapChain));
	g_uFrameIndex = g_pEngine->m_pSwapChain->GetCurrentBackBufferIndex();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create synchronization objects.
	ThrowIfFailed(g_pEngine->m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pEngine->m_pFence)));

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create an event handle to use for frame synchronization.
	g_pEngine->m_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (g_pEngine->m_hFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create RTV descriptor heaps.
	// Describe and create a render target view (RTV) descriptor heap.
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_uStart = 0;
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_uCount = APP_NUM_RTV + GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;

	D3D12_DESCRIPTOR_HEAP_DESC RHeapDesc = {};
	RHeapDesc.NumDescriptors = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_uCount;
	RHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&RHeapDesc, IID_PPV_ARGS(&(g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_pDescriptorHeap))));
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_RTV].m_uSize = g_pEngine->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DSV
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_uStart = 0;
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_uCount = 1;

	D3D12_DESCRIPTOR_HEAP_DESC DHeapDesc = {};
	DHeapDesc.NumDescriptors = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_uCount;
	DHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&DHeapDesc, IID_PPV_ARGS(&(g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_pDescriptorHeap))));
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Describe and create a depth stencil view (DSV) descriptor heap.
	// Create the depth stencil.
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_uSize = g_pEngine->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CreateDefaultDepthBuffer(g_pEngine->m_pDepthStencil.ReleaseAndGetAddressOf(), uWidth, uHeight, g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_DSV].m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// SRV
	// Describe and create a constant buffer view (CBV), Shader resource
	// view (SRV), and unordered access view (UAV) descriptor heap.
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_GCSU].m_uStart = 0;
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_GCSU].m_uCount = 64;

	D3D12_DESCRIPTOR_HEAP_DESC CSUHeapDesc = {};
	CSUHeapDesc.NumDescriptors = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_GCSU].m_uCount;
	CSUHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CSUHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&CSUHeapDesc, IID_PPV_ARGS(&(g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_GCSU].m_pDescriptorHeap))));
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_GCSU].m_uSize = g_pEngine->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_CCSU].m_uStart = 0;
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_CCSU].m_uCount = 5;

	//
	CSUHeapDesc.NumDescriptors = g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_CCSU].m_uCount;
	CSUHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CSUHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pEngine->m_pDevice->CreateDescriptorHeap(&CSUHeapDesc, IID_PPV_ARGS(&(g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_CCSU].m_pDescriptorHeap))));
	g_pEngine->m_hHandleHeap[XEngine::XDESCRIPTORHEAPTYPE_CCSU].m_uSize = g_pEngine->m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GraphicRoot
	{
		CD3DX12_DESCRIPTOR_RANGE granges[GRDT_COUNT];
		granges[GRDT_CBV_FRAMEBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);				// Content
		granges[GRDT_CBV_INSTANCEBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);			// Content
		granges[GRDT_SRV_TEXTURE].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);					// Texture
		granges[GRDT_SRV_GLOBALTEXTURE].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 3);			// Texture
		granges[GRDT_UVA_SBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);					// UAV
		granges[GRDT_SRV_POSTPROCESSTEXTURE].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);		// Texture

		CD3DX12_ROOT_PARAMETER grootParameters[GRDT_COUNT];
		grootParameters[GRDT_CBV_FRAMEBUFFER].InitAsDescriptorTable(1, &granges[GRDT_CBV_FRAMEBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		grootParameters[GRDT_CBV_INSTANCEBUFFER].InitAsDescriptorTable(1, &granges[GRDT_CBV_INSTANCEBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		grootParameters[GRDT_SRV_TEXTURE].InitAsDescriptorTable(1, &granges[GRDT_SRV_TEXTURE], D3D12_SHADER_VISIBILITY_ALL);
		grootParameters[GRDT_SRV_GLOBALTEXTURE].InitAsDescriptorTable(1, &granges[GRDT_SRV_GLOBALTEXTURE], D3D12_SHADER_VISIBILITY_ALL);
		grootParameters[GRDT_UVA_SBUFFER].InitAsDescriptorTable(1, &granges[GRDT_UVA_SBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		grootParameters[GRDT_SRV_POSTPROCESSTEXTURE].InitAsDescriptorTable(1, &granges[GRDT_SRV_POSTPROCESSTEXTURE], D3D12_SHADER_VISIBILITY_ALL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//D3D12_FILTER_MIN_MAG_MIP_LINEAR;//D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_ROOT_SIGNATURE_DESC grootSignatureDesc;
		grootSignatureDesc.Init(_countof(grootParameters), grootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&grootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(g_pEngine->m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_pEngine->m_pGraphicRootSignature)));
	}

	// ComputeRoot
	{
		CD3DX12_DESCRIPTOR_RANGE cranges[CRDT_COUNT];
		cranges[CRDT_SRV_TEXTURE].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);					// Texture
		cranges[CRDT_SRV_GLOBALTEXTURE].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 3);			// Texture
		cranges[CRDT_UVA_SRCSBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);				// UAV S
		cranges[CRDT_UVA_DSTSBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 1);				// UAV D
		cranges[CRDT_CBV_FRAMEBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);				// Content 0 
		cranges[CRDT_CBV_INSTANCEBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);			// Content 1

		CD3DX12_ROOT_PARAMETER crootParameters[CRDT_COUNT];
		crootParameters[CRDT_SRV_TEXTURE].InitAsDescriptorTable(1, &cranges[CRDT_SRV_TEXTURE], D3D12_SHADER_VISIBILITY_ALL);
		crootParameters[CRDT_SRV_GLOBALTEXTURE].InitAsDescriptorTable(1, &cranges[CRDT_SRV_GLOBALTEXTURE], D3D12_SHADER_VISIBILITY_ALL);
		crootParameters[CRDT_UVA_SRCSBUFFER].InitAsDescriptorTable(1, &cranges[CRDT_UVA_SRCSBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		crootParameters[CRDT_UVA_DSTSBUFFER].InitAsDescriptorTable(1, &cranges[CRDT_UVA_DSTSBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		crootParameters[CRDT_CBV_FRAMEBUFFER].InitAsDescriptorTable(1, &cranges[CRDT_CBV_FRAMEBUFFER], D3D12_SHADER_VISIBILITY_ALL);
		crootParameters[CRDT_CBV_INSTANCEBUFFER].InitAsDescriptorTable(1, &cranges[CRDT_CBV_INSTANCEBUFFER], D3D12_SHADER_VISIBILITY_ALL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		CD3DX12_ROOT_SIGNATURE_DESC crootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(crootParameters), crootParameters, 1, &sampler);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&crootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(g_pEngine->m_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_pEngine->m_pComputeRootSignature)));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Viewport & Scissor
	g_pEngine->m_Viewport.TopLeftX = 0;
	g_pEngine->m_Viewport.TopLeftY = 0;
	g_pEngine->m_Viewport.Width = static_cast<float>(uWidth);
	g_pEngine->m_Viewport.Height = static_cast<float>(uHeight);
	g_pEngine->m_Viewport.MaxDepth = 1.0f;
	g_pEngine->m_Viewport.MinDepth = 0.0f;
	g_pEngine->m_ScissorRect.left = 0;
	g_pEngine->m_ScissorRect.top = 0;
	g_pEngine->m_ScissorRect.right = static_cast<LONG>(uWidth);
	g_pEngine->m_ScissorRect.bottom = static_cast<LONG>(uHeight);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	g_pResourceThread = new XResourceThread();
	g_pResourceThread->Init(g_pEngine->m_pDevice);

	XBuffer::Init(g_pEngine->m_pDevice);
	XTextureSet::Init(g_pEngine->m_pDevice);

	//
	XFrameResource::Init(g_pEngine->m_pDevice);
	for (UINT n = 0; n < FRAME_NUM; n++)
	{
		g_pFrameResource[n] = new XFrameResource();
		g_pFrameResource[n]->InitInstance(n, g_pEngine->m_pDevice, g_pEngine->m_pSwapChain.Get());
	}

	// Framework
	InitDeferredShading(g_pEngine->m_pDevice,uWidth,uHeight);
	InitHDR(g_pEngine->m_pDevice, uWidth, uHeight);

	//
	InitAlphaRender(g_pEngine->m_pDevice, uWidth, uHeight);
	InitShadowMap(g_pEngine->m_pDevice, uWidth, uHeight);
	InitVoxelConeTracing(g_pEngine->m_pDevice, uWidth, uHeight);

	// PostProcess
	InitPostProcess(g_pEngine->m_pDevice, uWidth, uHeight);
	InitScreenSpaceReflection(g_pEngine->m_pDevice, uWidth, uHeight);
	InitSSAO(g_pEngine->m_pDevice, uWidth, uHeight);
	InitSMAA(g_pEngine->m_pDevice, uWidth, uHeight);

	//
	InitSkyBox(g_pEngine->m_pDevice, uWidth, uHeight);
	XEntity::Init(g_pEngine->m_pDevice);

	//
	//g_UIManager.Init(g_pEngine->m_pDevice.Get(),  uWidth, uHeight);

	return true;
}

bool Update()
{
	g_Timer.Tick(NULL);
	g_Camera.Update(static_cast<float>(g_Timer.GetElapsedSeconds()));
	XBuffer::Update();

	XMMATRIX matView = g_Camera.GetViewMatrix();
	XMMATRIX matProj = g_Camera.GetProjectionMatrix();
	g_pFrameResource[g_uFrameIndex]->UpdateConstantBuffers(matView, matProj);
	DeferredShading_Update(matView, matProj);

	//
	ShadowMap_Update(matView, matProj);

	//
	g_SceneGraph.Update();

	return true;
}

void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture);
bool Render()
{
	XFrameResource* pFrameResource = g_pFrameResource[g_uFrameIndex];
	ID3D12GraphicsCommandList *pCommandList = pFrameResource->m_pCommandList.Get();

	pFrameResource->Prepare();

	///////////////////////////////////////////////////////////////////////
	// ShadowMap
	ShadowMap_Begin(pCommandList);
	g_SceneGraph.Render(ERENDERPATH_SHADOWMAP, pCommandList, pFrameResource->m_uFenceValue);
	ShadowMap_End(pCommandList);

	///////////////////////////////////////////////////////////////////////
	// VoxelConeTracing
	VoxelConeTracing_Begin(pCommandList,0);
	g_SceneGraph.Render(ERENDERPATH_VOXEL, pCommandList, pFrameResource->m_uFenceValue);
	VoxelConeTracing_End(pCommandList,0);
	VoxelConeTracing_Begin(pCommandList, 1);
	g_SceneGraph.Render(ERENDERPATH_VOXEL, pCommandList, pFrameResource->m_uFenceValue);
	VoxelConeTracing_End(pCommandList, 1);
	VoxelConeTracing_Begin(pCommandList, 2);
	g_SceneGraph.Render(ERENDERPATH_VOXEL, pCommandList, pFrameResource->m_uFenceValue);
	VoxelConeTracing_End(pCommandList, 02);

	///////////////////////////////////////////////////////////////////////
	// FrameResource
	pFrameResource->PreRender();

	///////////////////////////////////////////////////////////////////////
	// DeferredShading
	// DeferredShading_GBuffer
	DeferredShading_GBuffer(pCommandList);
	g_SceneGraph.Render(ERENDERPATH_GEOMETRY,pCommandList,pFrameResource->m_uFenceValue);
	
	// HDR_Bind
	HDR_Bind(pCommandList);

	//
	SkyBox_Render(pCommandList);

	// DeferredShading_Shading
	DeferredShading_Shading(pCommandList);
	
	///////////////////////////////////////////////////////////////////////
	// ForwordShading
	// Alpha Blend
/*
	AlphaRender_Begin(pCommandList);
	g_SceneGraph.Render(ERENDERPATH_ALPHABLEND,pCommandList,pFrameResource->m_uFenceValue);
	AlphaRender_End(pCommandList);
*/

	//
	PostProcess_Bind(pCommandList);
	ScreenSpaceReflection_Render(pCommandList);
	
	//
	pFrameResource->BeginRender();
	HDR_ToneMapping(pCommandList);
	//g_UIManager.Render(pCommandList, sFrameResource.m_uFenceValue);

	//
	VoxelConeTracing_Render(pCommandList);
	
	// PostProcess
	SSAO_Render(pCommandList);
	SMAA_Render(pCommandList);
	pFrameResource->EndRender();

	///////////////////////////////////////////////////////////////////////
	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { pCommandList };
	g_pEngine->m_pRenderCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(g_pEngine->m_pSwapChain->Present(1, 0));
	MoveToNextFrame();
	return true;
}

void MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = g_pFrameResource[g_uFrameIndex]->m_uFenceValue;
	ThrowIfFailed(g_pEngine->m_pRenderCommandQueue->Signal(g_pEngine->m_pFence.Get(), currentFenceValue));

	// Update the frame index.
	g_uFrameIndex = g_uFrameIndex + 1;
	g_uFrameIndex = Mod<UINT>(g_uFrameIndex, FRAME_NUM);

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	UINT64 uCompleteValue = g_pEngine->m_pFence->GetCompletedValue();
	if (uCompleteValue < g_pFrameResource[g_uFrameIndex]->m_uFenceValue)
	{
		ThrowIfFailed(g_pEngine->m_pFence->SetEventOnCompletion(g_pFrameResource[g_uFrameIndex]->m_uFenceValue, g_pEngine->m_hFenceEvent));
		WaitForSingleObjectEx(g_pEngine->m_hFenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	g_pFrameResource[g_uFrameIndex]->m_uFenceValue = currentFenceValue + 1;
}

void WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(g_pEngine->m_pRenderCommandQueue->Signal(g_pEngine->m_pFence.Get(), g_pFrameResource[g_uFrameIndex]->m_uFenceValue));

	// Wait until the fence has been processed.
	ThrowIfFailed(g_pEngine->m_pFence->SetEventOnCompletion(g_pFrameResource[g_uFrameIndex]->m_uFenceValue, g_pEngine->m_hFenceEvent));
	WaitForSingleObjectEx(g_pEngine->m_hFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	//m_FrameResource[m_uFrameIndex].m_uFenceValue++;
}

void Clean()
{
	//
	WaitForGpu();

	//
	//g_UIManager.Clean();

	//
	for (UINT i = 0;i < FRAME_NUM;++i)
	{
		g_pFrameResource[i]->Clean();
	}

	// Framework
	CleanDeferredShading();
	CleanHDR();
	
	// Process
	CleanAlphaRender();
	CleanShadowMap();
	CleanVoxelConeTracing();

	// PostProcess
	CleanPostProcess();
	CleanScreenSpaceReflection();
	CleanSSAO();
	CleanSMAA();

	// Instance
	CleanSkyBox();
	g_SceneGraph.Clean();

	// Resource
	XGeometry::Clean();
	XBuffer::Clean();
	XTextureSet::Clean();

	//
	ID3D12Device *pDevice = g_pEngine->m_pDevice;
	SAFE_DELETE(g_pEngine);
#ifdef _DEBUG
	ID3D12DebugDevice *pd3dDebugDevice = nullptr;
	pDevice->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(&pd3dDebugDevice));
	if (pd3dDebugDevice)
	{
		//pd3dDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		pd3dDebugDevice->Release();
	}
#endif
	pDevice->Release();

	//
	for (UINT i = 0;i < FRAME_NUM;++i)
	{
		SAFE_DELETE(g_pFrameResource[i]);
	}
	SAFE_DELETE(g_pResourceThread);
}

std::vector<PointLight> vPointLight;
void AddPointLight(PointLight& sPointLight)
{
	if (vPointLight.size() >= LIGHT_MAXNUM)
		return;

	vPointLight.push_back(sPointLight);
}
PointLight* GetPointLight(UINT uIndex)
{
	if (uIndex >= vPointLight.size())
		return nullptr;

	return &vPointLight[uIndex];
}