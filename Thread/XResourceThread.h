
#pragma once

#include "..\XDirectX12.h"
#include <DirectXMath.h>
using namespace DirectX;

class IResourceLoad
{
public:
	virtual ~IResourceLoad() {};
	virtual void LoadFromFile() = 0;
	virtual void PostLoad() = 0;
	virtual bool IsNeedWaitForResource() = 0;
};

//
class XResourceThread
{
	//
	HANDLE								m_hThread;
	std::list<IResourceLoad*>			m_lTask;

	//
	ComPtr<ID3D12CommandQueue>			m_pResourceCommandQueue;
	ComPtr<ID3D12CommandAllocator>		m_pResourceCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>	m_pResourceCommandList;

	HANDLE								m_hFenceEvent;
	ComPtr<ID3D12Fence>					m_pFence;
	UINT64								m_uFenceValue;

public:
	XResourceThread();
	~XResourceThread();

	void Init(ID3D12Device* pDevice);
	void Work();
	void Load();

	virtual bool InsertResourceLoadTask(IResourceLoad *pResourceLoad);
	//bool InsertTextureLoadTask(LPCWSTR pFileName, Texture *pTexture, UINT uCount, LPCWSTR pDetailName[], ResourceSet* pResourceSet);
	//bool InsertTextureLoadTask(LPCWSTR pFileName, Texture *pTexture, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter, ResourceSet* pResourceSet);
	//bool InsertShaderLoadTask(LPCWSTR pFileName, Shader *pShader, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, ResourceSet *pResourceSet);
	//bool InsertSkeletonLoadTask(LPCWSTR pFileName,Skeleton *pSkeleton, ResourceSet* pResourceSet);
	//bool InsertAnimateLoadTask(LPCWSTR pFileName, Animate *pAnimate, ResourceSet* pResourceSet);

	//
	HANDLE GetHandle();
	void WaitForResource();
	ID3D12CommandQueue* GetResourceCommandQueue();
	ID3D12GraphicsCommandList* GetResourceCommandList();
};