
#pragma once

#include "XResource.h"

#include "..\XDirectX12.h"
#include "..\Thread\XResourceThread.h"

#include <Wincodec.h>
#include <map>

//
struct XTextureSet : public XResource
{
	enum eTextureType
	{
		ETEXTURETYPE_2D = 0,
		ETEXTURETYPE_3D,
		ETEXTURETYPE_CUBE
	};

	std::vector<ID3D12Resource*>			m_vpTexture;
	UINT									m_uSBaseIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE				m_hSRVCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE				m_hSRVGpuHandle;
public:
	XTextureSet(LPCWSTR pName,UINT uSBaseIndedx) :XResource(pName),m_uSBaseIndex(uSBaseIndedx){}
	~XTextureSet();

	void Release();
	virtual UINT GetSBaseIndex() { return m_uSBaseIndex; }
	UINT GetTextureCount() { return (UINT)m_vpTexture.size(); }

	D3D12_CPU_DESCRIPTOR_HANDLE& GetSRVCpuHandle()
	{
		return m_hSRVCpuHandle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGpuHandle()
	{
		return m_hSRVGpuHandle;
	}

private:
	static IWICImagingFactory						*m_pWIC;
public:
	static void Init(ID3D12Device* pDevice);
	static void Clean();
	static IWICImagingFactory *GetImagingFactory() { return m_pWIC; }
};
class XTextureSetManager : public XResourceManager<XTextureSet>
{
	static XTextureSet* CreateTextureSet(LPCWSTR pName, UINT uCount, LPCWSTR pFileName[], XTextureSet::eTextureType eType[], UINT uSRVIndex);
public:
	static XTextureSet* CreateTextureSet(LPCWSTR pName, UINT uCount, LPCWSTR pFileName[], UINT uSRVIndex);
	static XTextureSet* CreateCubeTexture(LPCWSTR pName, LPCWSTR pFileName, UINT uSRVIndex);
	static XTextureSet* CreateTextureSet(LPCWSTR pName, UINT uSRVIndex, UINT uWidth, UINT uHeight, DXGI_FORMAT Format, UINT8 *pData, UINT uPixelSize);
};

struct STextureLayer
{
	std::wstring							m_sFileName;
	DXGI_FORMAT								m_Format;
	XTextureSet::eTextureType				m_eType;
	UINT									m_uWidth, m_uHeight, m_uPixelSize;
	UINT8									*m_pData;
	
	ComPtr<ID3D12Resource>					m_pTextureUpload;

	STextureLayer() :m_eType(XTextureSet::ETEXTURETYPE_2D),m_pData(nullptr) {}
	~STextureLayer()
	{
		SAFE_DELGRP(m_pData);
	}
};

//typedef UINT8* (*CreateTextureFun)(UINT uWidth, UINT uHeight, UINT uPixelSize, UINT uParameter);
struct TextureLoad : public IResourceLoad
{
	UINT									m_uIndex;
	STextureLayer							m_TextureLayer;
	XTextureSet								*m_pTextureSet;

	TextureLoad() :m_uIndex(0) {};
	virtual bool IsNeedWaitForResource() { return true; };
};

struct TextureSetLoad : public IResourceLoad
{
	std::vector<TextureLoad*>				m_vTextureLayer;

	XTextureSet								*m_pTextureSet;
	UINT									m_uParameter;

	//
	TextureSetLoad() :m_uParameter(0) {}//m_pFun(nullptr), 
	~TextureSetLoad();

	virtual void LoadFromFile();
	virtual void PostLoad();
	virtual bool IsNeedWaitForResource() { return true; };
};

struct FileTextureLoad : public TextureLoad
{
	virtual void LoadFromFile();
	virtual void PostLoad();
};

struct DDSFileTextureLoad : public FileTextureLoad
{
	virtual void LoadFromFile();
	virtual void PostLoad();
};

struct MemoryTextureLoad : public FileTextureLoad
{
	virtual void LoadFromFile();
	virtual void PostLoad();
};

class XRenderTarget
{
	ComPtr<ID3D12Resource>				m_pRenderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_hRTVCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_hRTVGpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_hSRVCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_hSRVGpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_hUAVCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_hUAVGpuHandle;
public:
	ID3D12Resource* GetResource() { return m_pRenderTarget.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE& GetRTVCpuHandle()
	{
		return m_hRTVCpuHandle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE& GetRTVGpuHandle()
	{
		return m_hRTVGpuHandle;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE& GetSRVCpuHandle()
	{
		return m_hSRVCpuHandle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGpuHandle()
	{
		return m_hSRVGpuHandle;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE& GetUAVCpuHandle()
	{
		return m_hUAVCpuHandle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGpuHandle()
	{
		return m_hUAVGpuHandle;
	}

	static XRenderTarget* CreateRenderTarget(DXGI_FORMAT Format, UINT uWidth, UINT uHeight, UINT uRTVIndex, UINT uSRVIndex, UINT uUAVIndex = 0xFFFFFFFF);
};