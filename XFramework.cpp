
#include <Windows.h>
//#include "XFramework.h"
#include "XDirectX12.h"
#include "XCamera.h"
#include "XEntity.h"
#include "UI\UIManager.h"
#include "Loader\XBinLoader.h"
#include "Thread\XResourceThread.h"

extern XCamera			g_Camera;
extern XEntity			*g_pEntityNormal;
extern XEntity			*g_pEntityAlpha;

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

	MyRegisterClass(hInstance);
	HWND hWnd = CreateWindowW(L"Class", L"XFramework", WS_OVERLAPPEDWINDOW,
		0, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);

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
	//g_ResourceThread.WaitForResource();

	//
	g_Camera.Init(0.8f, 1.0f);

	//
	PointLight pp;
	pp.fPosX = 0.0f;
	pp.fPosY = 5.0f;
	pp.fPosZ = 0.0f;
	pp.fAttenuationEnd = 10.0f;
	pp.fAttenuationBegin = 0.0f;
	pp.fR = 1.0f;
	pp.fG = 0.0f;
	pp.fB = 0.0f;
	AddPointLight(pp);

	pp.fPosX = 5.0f;
	pp.fR = 0.0f;
	pp.fG = 1.0f;
	AddPointLight(pp);

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