
#include "XDirectX12.h"
#include "XTilebaseDeferredShading.h"
#include "XOrderIndependentTransparency.h"

#include "Thread\XResourceThread.h"
#include "Resource\XFrameResource.h"
#include "Resource\XBufferManager.h"
#include "Resource\XTextureManager.h"
#include "UI\UIManager.h"
#include "XEntity.h"
#include "XCamera.h"
#include "StepTimer.h"
#include "Loader\XBinLoader.h"

#include <d2d1_3.h>
#include <dwrite.h>

#include <D3Dcompiler.h>
#include "d3dx12.h"
#include <DirectXMath.h>
#include <d3d11on12.h>
#include "DXSampleHelper.h"

UINT g_uRenderTargetCount[ESHADINGPATH_COUNT] = { 1,3 };
DXGI_FORMAT g_RenderTargetFortmat[ESHADINGPATH_COUNT][RENDERTARGET_MAXNUM] =
{
	{ DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R8G8B8A8_UNORM },
	{ DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R8G8B8A8_UNORM },
	//{ DXGI_FORMAT_R16G16B16A16_FLOAT ,DXGI_FORMAT_R8G8B8A8_UNORM ,DXGI_FORMAT_R16G16_FLOAT }
};

HWND								g_hWnd;
ComPtr<ID3D12Device>				g_pDevice;
ComPtr<IDXGISwapChain3>				g_pSwapChain;
ComPtr<ID3D12CommandQueue>			g_pRenderCommandQueue;
ComPtr<ID3D12RootSignature>			g_pRootSignature;

UINT								g_uFrameIndex;
HANDLE								g_hFenceEvent;
ComPtr<ID3D12Fence>					g_pFence;
XFrameResource						g_FrameResource[3];

//
ComPtr<ID3D12DescriptorHeap>		g_pRDescriptorHeap;
UINT								g_uRDescriptorSize;

ComPtr<ID3D12DescriptorHeap>		g_pDDescriptorHeap;
ComPtr<ID3D12Resource>				g_pDepthStencil;

ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
UINT								g_uCSUDescriptorSize;

extern XResourceThread				g_ResourceThread;
extern XBufferManager				g_BufferManager;
extern XTextureManager				*g_pTextureManager;
extern UIManager					g_UIManager;
extern XCamera						g_Camera;
StepTimer							g_Timer;

//
D3D12_VIEWPORT						g_Viewport;
D3D12_RECT							g_ScissorRect;

XEntity								*g_pEntity = nullptr;
bool CreateDevice(HWND hWnd, UINT uWidth, UINT uHeight, bool bWindow)
{
	g_hWnd = hWnd;
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#ifdef _DEBUG
/*
	// Enable the D3D12 debug layer.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
*/
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
			IID_PPV_ARGS(&g_pDevice)
			));
	}

	// Describe and create the command queues.
	D3D12_COMMAND_QUEUE_DESC renderqueueDesc = {};
	renderqueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	renderqueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_pDevice->CreateCommandQueue(&renderqueueDesc, IID_PPV_ARGS(&g_pRenderCommandQueue)));

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
		g_pRenderCommandQueue.Get(),		// Swap chain needs the render queue so that it can force a flush on it.
		&swapChainDesc,
		&swapChain
		));

	ThrowIfFailed(swapChain.As(&g_pSwapChain));
	g_uFrameIndex = g_pSwapChain->GetCurrentBackBufferIndex();

	// Create synchronization objects.
	ThrowIfFailed(g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pFence)));

	// Create an event handle to use for frame synchronization.
	g_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (g_hFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Create descriptor heaps.
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC RHeapDesc = {};
	// 3 for FrameSource RenderTarget,3 for DeferredShading RenderTarget
	RHeapDesc.NumDescriptors = 3+3;
	RHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pDevice->CreateDescriptorHeap(&RHeapDesc, IID_PPV_ARGS(&g_pRDescriptorHeap)));

	g_uRDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC DHeapDesc = {};
	DHeapDesc.NumDescriptors = 1;
	DHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_pDevice->CreateDescriptorHeap(&DHeapDesc, IID_PPV_ARGS(&g_pDDescriptorHeap)));

	// Create the depth stencil.
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(g_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, uWidth, uHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&g_pDepthStencil)
		));

	g_pDevice->CreateDepthStencilView(g_pDepthStencil.Get(), &depthStencilDesc, g_pDDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//
	// Describe and create a constant buffer view (CBV), Shader resource
	// view (SRV), and unordered access view (UAV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC CSUHeapDesc = {};
	// 3 for FrameResource ContentBuffer,3 for DeferredShading RenderTarget ShaderView,3 For OIT UAV,2 For Entity's Texture
	CSUHeapDesc.NumDescriptors = 3 + 3 + 3 + 2;
	CSUHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CSUHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(g_pDevice->CreateDescriptorHeap(&CSUHeapDesc, IID_PPV_ARGS(&g_pCSUDescriptorHeap)));

	//
	g_uCSUDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//
	CD3DX12_DESCRIPTOR_RANGE ranges[4];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);			// Content
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);			// Content
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);			// Texture
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);			// UAV

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(g_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_pRootSignature)));

	//
	g_Viewport.Width = static_cast<float>(uWidth);
	g_Viewport.Height = static_cast<float>(uHeight);
	g_Viewport.MaxDepth = 1.0f;
	g_ScissorRect.right = static_cast<LONG>(uWidth);
	g_ScissorRect.bottom = static_cast<LONG>(uHeight);

	//
	for (UINT n = 0; n < FRAME_NUM; n++)
	{
		g_FrameResource[n].InitInstance(n, g_pDevice.Get(), g_pSwapChain.Get());
	}
	InitDeferredShading(g_pDevice.Get(),uWidth,uHeight);
	InitOrderIndependentTransparency(g_pDevice.Get(), uWidth, uHeight);

	g_ResourceThread.Init(g_pDevice.Get());
	g_BufferManager.Init(g_pDevice.Get());
	g_pTextureManager = new XTextureManager;
	g_pTextureManager->Init(g_pDevice.Get());
	g_UIManager.Init(g_pDevice.Get(),  uWidth, uHeight);

	/*
		// Create an 11 device wrapped around the 12 device and share
		// 12's command queue.
		ComPtr<ID3D11Device> d3d11Device;
		ThrowIfFailed(D3D11On12CreateDevice(
			g_pDevice.Get(),
			d3d11DeviceFlags,
			nullptr,
			0,
			reinterpret_cast<IUnknown**>(g_pRenderCommandQueue.GetAddressOf()),
			1,
			0,
			&d3d11Device,
			&m_DirectX11Layer.m_pDeviceContext,
			nullptr
			));

		// Query the 11On12 device from the 11 device.
		ThrowIfFailed(d3d11Device.As(&m_DirectX11Layer.m_p11On12Device));

		// Create D2D/DWrite components.
		{
			D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
			ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_DirectX11Layer.m_d2dFactory));
			ComPtr<IDXGIDevice> dxgiDevice;
			ThrowIfFailed(m_DirectX11Layer.m_p11On12Device.As(&dxgiDevice));
			ThrowIfFailed(m_DirectX11Layer.m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_DirectX11Layer.m_d2dDevice));
			ThrowIfFailed(m_DirectX11Layer.m_d2dDevice->CreateDeviceContext(deviceOptions, &m_DirectX11Layer.m_d2dDeviceContext));
			ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_DirectX11Layer.m_dWriteFactory));
		}

		// Query the desktop's dpi settings, which will be used to create
		// D2D's render targets.
		float dpiX;
		float dpiY;
		m_DirectX11Layer.m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpiX,
			dpiY
			);

		// Create D2D/DWrite objects for rendering text.
		{
			ThrowIfFailed(m_DirectX11Layer.m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_DirectX11Layer.m_textBrush));
			ThrowIfFailed(m_DirectX11Layer.m_dWriteFactory->CreateTextFormat(
				L"Verdana",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				50,
				L"en-us",
				&m_DirectX11Layer.m_textFormat
				));
			ThrowIfFailed(m_DirectX11Layer.m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			ThrowIfFailed(m_DirectX11Layer.m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
		}
	*/

	//
	//m_Viewport.Width = static_cast<float>(uWidth);
	//m_Viewport.Height = static_cast<float>(uHeight);
	//m_Viewport.MaxDepth = 1.0f;
	//m_ScissorRect.right = static_cast<LONG>(uWidth);
	//m_ScissorRect.bottom = static_cast<LONG>(uHeight);

	return true;
}

bool Update()
{
	g_Timer.Tick(NULL);
	g_Camera.Update(static_cast<float>(g_Timer.GetElapsedSeconds()));

	g_FrameResource[g_uFrameIndex].UpdateConstantBuffers(g_Camera.GetViewMatrix(), g_Camera.GetProjectionMatrix());
	return true;
}

bool Render()
{
	XFrameResource& sFrameResource = g_FrameResource[g_uFrameIndex];
	ID3D12GraphicsCommandList *pCommandList = sFrameResource.m_pCommandList.Get();

	// FrameResource
	sFrameResource.PreRender();

	// DeferredShading
	BeginDeferredShading(pCommandList);
	if (g_pEntity)
	{
		g_pEntity->Render(pCommandList, sFrameResource.m_uFenceValue);
	}
	EndDeferredShading(pCommandList);

	// FrameResource
	sFrameResource.BeginRender();

	//

	// Alpha Blend
	BeginOrderIndependentTransparency(pCommandList);
	g_UIManager.Render(pCommandList, sFrameResource.m_uFenceValue);
	EndOrderIndependentTransparency(pCommandList);

	// AddAll

	//
	sFrameResource.EndRender();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { pCommandList };
	g_pRenderCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(g_pSwapChain->Present(1, 0));
	MoveToNextFrame();
	return true;
}

void MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = g_FrameResource[g_uFrameIndex].m_uFenceValue;
	ThrowIfFailed(g_pRenderCommandQueue->Signal(g_pFence.Get(), currentFenceValue));

	// Update the frame index.
	g_uFrameIndex = g_pSwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	UINT64 uCompleteValue = g_pFence->GetCompletedValue();
	if (uCompleteValue < g_FrameResource[g_uFrameIndex].m_uFenceValue)
	{
		ThrowIfFailed(g_pFence->SetEventOnCompletion(g_FrameResource[g_uFrameIndex].m_uFenceValue, g_hFenceEvent));
		WaitForSingleObjectEx(g_hFenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	g_FrameResource[g_uFrameIndex].m_uFenceValue = currentFenceValue + 1;
}

void WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(g_pRenderCommandQueue->Signal(g_pFence.Get(), g_FrameResource[g_uFrameIndex].m_uFenceValue));

	// Wait until the fence has been processed.
	ThrowIfFailed(g_pFence->SetEventOnCompletion(g_FrameResource[g_uFrameIndex].m_uFenceValue, g_hFenceEvent));
	WaitForSingleObjectEx(g_hFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	//m_FrameResource[m_uFrameIndex].m_uFenceValue++;
}

void Clean()
{
	WaitForGpu();

	//
	SAFE_DELETE(g_pTextureManager);
	SAFE_DELETE(g_pEntity);
	CleanDeferredShading();
	CleanOrderIndependentTransparency();
}

//
XGeometry *pFullScreenGeometry = nullptr;
class FullScreenResource : public IResourceLoad
{
public:
	virtual void LoadFromFile()
	{
		//
		struct Vertex
		{
			DirectX::XMFLOAT4 position;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 uv;
		};
		Vertex triangleVertices[] =
		{
			{ { -1.00f,  1.00f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
			{ { 1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },

			{ { -1.00f,  1.00f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 1.00f, -1.00f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 1.00f,  1.00f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		};
		UINT uIndex[] = { 0,1,2,3,4,5 };

		UINT8 *pData = new UINT8[6 * sizeof(Vertex) + 6 * sizeof(UINT)];
		UINT8 *pVertexData = pData;
		memcpy(pVertexData, &triangleVertices[0], 6 * sizeof(Vertex));
		UINT8 *pIndexData = pData + 6 * sizeof(Vertex);
		memcpy(pIndexData, &uIndex[0], 6 * sizeof(UINT));

		XGeometry *pGeometry = CreateGeometry(6, sizeof(Vertex), 6, DXGI_FORMAT_R32_UINT, pData);//dynamic_cast<Geometry*>(GetXEngine()->GetGeometryManager()->CreateGeometry(L"UIGeometry"));
		if (pGeometry)
		{
			pFullScreenGeometry = pGeometry;
		}
		delete[] pData;
	}
	virtual void PostLoad()
	{
		//pUIManager->IncreaseResourceComplate();
	}
	virtual bool IsNeedWaitForResource()
	{
		return true;
	}
};
void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList,XShader *pShader)
{
	if (!pFullScreenGeometry)
	{
		FullScreenResource *pResource = new FullScreenResource();
		g_ResourceThread.InsertResourceLoadTask(pResource);
	}

	//
	pCommandList->SetPipelineState(pShader->GetPipelineState());
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//
	pCommandList->IASetVertexBuffers(0, 1, pFullScreenGeometry->GetVertexBufferView());
	if (pFullScreenGeometry->GetNumIndices())
	{
		pCommandList->IASetIndexBuffer(pFullScreenGeometry->GetIndexBufferView());
		pCommandList->DrawIndexedInstanced(pFullScreenGeometry->GetNumIndices(), 1, 0, 0, 0);
	}
}