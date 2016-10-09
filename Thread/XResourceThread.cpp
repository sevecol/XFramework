

#include <process.h>

#include "XResourceThread.h"
#include "..\DXSampleHelper.h"

CRITICAL_SECTION	g_cs;
XResourceThread		g_ResourceThread;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XResourceThread::XResourceThread():m_uFenceValue(0)
{
	InitializeCriticalSection(&g_cs);
}
XResourceThread::~XResourceThread()
{
	CloseHandle(m_hThread);
	CloseHandle(m_hFenceEvent);
}

void XResourceThread::Init(ID3D12Device* pDevice)
{
	D3D12_COMMAND_QUEUE_DESC renderqueueDesc = {};
	renderqueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	renderqueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(pDevice->CreateCommandQueue(&renderqueueDesc, IID_PPV_ARGS(&m_pResourceCommandQueue)));
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pResourceCommandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pResourceCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pResourceCommandList)));
	//ThrowIfFailed(m_pCommandList->Close());

	// Create synchronization objects.
	ThrowIfFailed(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
	m_uFenceValue = 1;

	// Create an event handle to use for frame synchronization.
	m_hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_hFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	//
	struct threadwrapper
	{
		static unsigned int WINAPI thunk(LPVOID lpParameter)
		{
			g_ResourceThread.Work();
			return 0;
		}
	};
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		0,
		threadwrapper::thunk,
		0,
		CREATE_SUSPENDED,
		nullptr));
}

void XResourceThread::Work()
{
	while (1)
	{
		if (m_lTask.size() == 0)
		{
			SuspendThread(m_hThread);
		}

		EnterCriticalSection(&g_cs);
		IResourceLoad *pResourceLoad = *(m_lTask.begin());
		m_lTask.pop_front();
		LeaveCriticalSection(&g_cs);

		if (pResourceLoad)
		{
			pResourceLoad->LoadFromFile();
			pResourceLoad->PostLoad();

			//
			if (pResourceLoad->IsNeedWaitForResource())
			{
				WaitForResource();
			}
		}

		//
		delete pResourceLoad;
	}
}
void XResourceThread::Load()
{
/*
	if (m_lTask.size() == 0)
	{
		return;
	}

	IResource *pResource = *(m_lTask.begin());
	m_lTask.pop_front();

	if (pResource)
	{
		pResource->LoadFromFile();
		pResource->PostLoad();

		//
		if (pResource->IsNeedWaitForResource())
		{
			WaitForResource();
		}
	}

	//
	delete pResource;
*/
}

bool XResourceThread::InsertResourceLoadTask(IResourceLoad *pResourceLoad)
{
	EnterCriticalSection(&g_cs);
	m_lTask.push_back(pResourceLoad);
	LeaveCriticalSection(&g_cs);
	ResumeThread(m_hThread);

	return true;
}
/*
bool XResourceThread::InsertTextureLoadTask(LPCWSTR pFileName, Texture *pTexture, UINT uCount, LPCWSTR pDetailName[], ResourceSet* pResourceSet)
{
	//
	if (!pTexture)
	{
		return false;
	}
	pTexture->m_eState = ERESOURCESTATE_LOADING;

	//
	TextureLoad *pTextureLoad = new TextureLoad();
	pTextureLoad->m_pFileName	= pFileName;
	pTextureLoad->m_pTexture	= pTexture;
	pTextureLoad->m_pFun		= nullptr;
	pTextureLoad->m_uParameter	= 0;
	pTextureLoad->m_pResourceSet= pResourceSet;

	for (UINT i = 0;i < uCount;++i)
	{
		STextureLayer sTextureLayer;
		sTextureLayer.m_sFileName = std::wstring(pFileName) + pDetailName[i];
		pTextureLoad->m_vTextureLayer.push_back(sTextureLayer);
	}
	
	EnterCriticalSection(&g_cs);
	m_lTask.push_back(pTextureLoad);
	LeaveCriticalSection(&g_cs);
	ResumeThread(m_hThread);

	return true;
}
bool XResourceThread::InsertTextureLoadTask(LPCWSTR pFileName, Texture *pTexture, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter, ResourceSet* pResourceSet)
{
	//
	if (!pTexture)
	{
		return false;
	}
	pTexture->m_eState = ERESOURCESTATE_LOADING;

	//
	TextureLoad *pTextureLoad	= new TextureLoad();
	pTextureLoad->m_pFileName	= pFileName;
	pTextureLoad->m_pTexture	= pTexture;

	STextureLayer sTextureLayer;
	sTextureLayer.m_uWidth		= uWidth;
	sTextureLayer.m_uHeight		= uHeight;
	sTextureLayer.m_uPixelSize	= uPixelSize;
	pTextureLoad->m_vTextureLayer.push_back(sTextureLayer);

	pTextureLoad->m_pFun = pFun;
	pTextureLoad->m_uParameter = uParameter;
	pTextureLoad->m_pResourceSet = pResourceSet;

	EnterCriticalSection(&g_cs);
	m_lTask.push_back(pTextureLoad);
	LeaveCriticalSection(&g_cs);
	ResumeThread(m_hThread);

	return true;
}
bool XResourceThread::InsertShaderLoadTask(LPCWSTR pFileName, Shader *pShader, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, ResourceSet *pResourceSet)
{
	//
	if (!pShader)
	{
		return false;
	}
	pShader->m_eState = ERESOURCESTATE_LOADING;

	ShaderLoad *pShaderLoad = new ShaderLoad();
	pShaderLoad->m_pFileName			= pFileName;
	pShaderLoad->m_pShader				= pShader;
	pShaderLoad->m_pVSEntryPoint		= pVSEntryPoint;
	pShaderLoad->m_pVSTarget			= pVSTarget;
	pShaderLoad->m_pPSEntryPoint		= pPSEntryPoint;
	pShaderLoad->m_pPSTarget			= pPSTarget;
	pShaderLoad->m_uInputElementCount	= uInputElementCount;
	pShaderLoad->m_pInputElementDescs	= new D3D12_INPUT_ELEMENT_DESC[uInputElementCount];
	memcpy(&(pShaderLoad->m_pInputElementDescs[0]), &(InputElementDescs[0]), sizeof(D3D12_INPUT_ELEMENT_DESC)*uInputElementCount);
	pShaderLoad->m_pResourceSet			= pResourceSet;

	EnterCriticalSection(&g_cs);
	m_lTask.push_back(pShaderLoad);
	LeaveCriticalSection(&g_cs);
	ResumeThread(m_hThread);

	return true;
}
*/
HANDLE XResourceThread::GetHandle()
{
	return m_hThread;
}
void XResourceThread::WaitForResource()
{
	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(m_pResourceCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_pResourceCommandList.Get() };
	m_pResourceCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_uFenceValue;
	ThrowIfFailed(m_pResourceCommandQueue->Signal(m_pFence.Get(), fence));
	m_uFenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_pFence->SetEventOnCompletion(fence, m_hFenceEvent));
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

	//
	ThrowIfFailed(m_pResourceCommandAllocator->Reset());
	ThrowIfFailed(m_pResourceCommandList->Reset(m_pResourceCommandAllocator.Get(), nullptr));
}

ID3D12CommandQueue* XResourceThread::GetResourceCommandQueue()
{
	return m_pResourceCommandQueue.Get();
}
ID3D12GraphicsCommandList* XResourceThread::GetResourceCommandList()
{
	return m_pResourceCommandList.Get();
}