
#include <Windows.h>
//#include "XFramework.h"
#include "XDirectX12.h"
#include "XCamera.h"
#include "XEntity.h"
#include "UI\UIManager.h"
#include "Loader\XBinLoader.h"
#include "Loader\XObjLoader.h"
#include "Thread\XResourceThread.h"

extern XCamera			g_Camera;
extern XEntity			*g_pEntityNormal;
extern XEntity			*g_pEntityAlpha;
extern XEntity			*g_pEntityPBRL;
extern XEntity			*g_pEntityPBRC;
extern XEntity			*g_pEntityPBRR;

//extern UIManager		g_UIManager;
extern XResourceThread	*g_pResourceThread;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE hInst;                                // 当前实例

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 执行应用程序初始化: 
	CoInitialize(NULL);
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// 主消息循环: 
	while (true)
	//while (GetMessage(&msg, nullptr, 0, 0))
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

		//
		PointLight* pLight = GetPointLight(0);
		static float fTime = 0.0f;
		fTime += 0.01f;

		if (pLight)
		{
			pLight->fPosX = 13.0f * sinf(fTime);
			pLight->fPosY = 13.0f;
			pLight->fPosZ = 13.0f * cosf(fTime);
		}

		Update();
		Render();
	}

	//
	//CoUninitialize();
	Clean();
	CoUninitialize();

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;//MAKEINTRESOURCEW(IDC_GAME);
	wcex.lpszClassName = L"Class";
	wcex.hIconSm = nullptr;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	MyRegisterClass(hInstance);
	HWND hWnd = CreateWindowW(L"Class", L"XFramework", WS_OVERLAPPEDWINDOW,
		(width-1280)/2.0f,(height-720)/2.0f, 1280, 720, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//
	RECT winrect, clientrect;
	GetWindowRect(hWnd, &winrect);
	GetClientRect(hWnd, &clientrect);

	//
	bool bResult = CreateDevice(hWnd, 1280, 720, true);

	//
	//g_UIManager.CreateUIImgWindow(nullptr, L"", 100, 100, 100, 100);

	//
	{
		g_pEntityNormal = new XEntity();

		D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		g_pEntityNormal->InitShader(L"shaders_entity_ds.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4, ESHADINGPATH_DEFERRED);

		//LPCWSTR pTextureFileName[2] = { L"terrain.png",L"wings.bmp" };
		//g_pEntityNormal->InitTexture(L"NormalEntity", 2, pTextureFileName,XTextureSet::ETEXTUREFILETYPE_OTHER);

		XBinResource *pbinresource = new XBinResource();
		pbinresource->pEntity = g_pEntityNormal;
		g_pResourceThread->InsertResourceLoadTask(pbinresource);
	}
	{
		g_pEntityAlpha = new XEntity();

		D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		g_pEntityAlpha->InitShader(L"shaders_entity_alpha.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4);

		//LPCWSTR pTextureFileName[2] = { L"terrain.png",L"wings.bmp" };
		//g_pEntityAlpha->InitTexture(L"AlphaEntity", 2, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		XBinResource *pbinresource = new XBinResource();
		pbinresource->pEntity = g_pEntityAlpha;
		g_pResourceThread->InsertResourceLoadTask(pbinresource);
	}
	{
		g_pEntityPBRC = new XEntity();

		D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		g_pEntityPBRC->InitShader(L"shaders_entity_ds.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4, ESHADINGPATH_DEFERRED);

		LPCWSTR pTextureFileName[3] = { L"albedo_stone.jpg",L"normal.jpg",L"mask_nonmetal.jpg" };
		g_pEntityPBRC->InitTexture(L"EntityPBRC", 3, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		//LPCWSTR pTextureFileName[2] = { L"terrain.png",L"wings.bmp" };
		//g_pEntityAlpha->InitTexture(L"AlphaEntity", 2, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		XObjResource *pobjresource = new XObjResource();
		pobjresource->pEntity = g_pEntityPBRC;
		g_pResourceThread->InsertResourceLoadTask(pobjresource);
	}
	{
		g_pEntityPBRL = new XEntity();

		D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		g_pEntityPBRL->InitShader(L"shaders_entityl_ds.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4, ESHADINGPATH_DEFERRED);

		LPCWSTR pTextureFileName[3] = { L"albedo_silver.jpg",L"normal.jpg",L"mask_metal.jpg" };
		g_pEntityPBRL->InitTexture(L"EntityPBRL", 3, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		//LPCWSTR pTextureFileName[2] = { L"terrain.png",L"wings.bmp" };
		//g_pEntityAlpha->InitTexture(L"AlphaEntity", 2, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		XObjResource *pobjresource = new XObjResource();
		pobjresource->pEntity = g_pEntityPBRL;
		g_pResourceThread->InsertResourceLoadTask(pobjresource);
	}
	{
		g_pEntityPBRR = new XEntity();

		D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		g_pEntityPBRR->InitShader(L"shaders_entityr_ds.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", StandardVertexDescription, 4, ESHADINGPATH_DEFERRED);

		LPCWSTR pTextureFileName[3] = { L"albedo_gold.jpg",L"normal.jpg",L"mask_metal.jpg" };
		g_pEntityPBRR->InitTexture(L"EntityPBRR", 3, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		//LPCWSTR pTextureFileName[2] = { L"terrain.png",L"wings.bmp" };
		//g_pEntityAlpha->InitTexture(L"AlphaEntity", 2, pTextureFileName, XTextureSet::ETEXTUREFILETYPE_OTHER);

		XObjResource *pobjresource = new XObjResource();
		pobjresource->pEntity = g_pEntityPBRR;
		g_pResourceThread->InsertResourceLoadTask(pobjresource);
	}
	//g_ResourceThread.WaitForResource();

	//
	g_Camera.Init(0.8f, 1280.0f/720.0f);
	g_Camera.InitPos(XMFLOAT3(0, 0, 24));

	//
	PointLight pp;
	pp.fPosX = 13.0f;
	pp.fPosY = 13.0f;
	pp.fPosZ = 0.0f;
	pp.fAttenuationEnd = 40.0f;
	pp.fAttenuationBegin = 0.0f;
	pp.fR = 2.0f;
	pp.fG = 2.0f;
	pp.fB = 2.0f;
	AddPointLight(pp);

	pp.fPosX = 13.0f;
	pp.fR = 0.0f;
	pp.fG = 2.0f;
	//AddPointLight(pp);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	g_Camera.ProcessMessage(message, wParam, lParam);
	switch (message)
	{
	case WM_COMMAND:
	{
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}