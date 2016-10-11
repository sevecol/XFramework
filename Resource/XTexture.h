
#pragma once

#include "..\XDirectX12.h"
#include "..\Thread\XResourceThread.h"

//
struct XTextureSet
{
	std::vector<ID3D12Resource*>			m_vpTexture;
	UINT									m_uSBaseIndex;
public:
	XTextureSet(UINT uSBaseIndedx) :m_uSBaseIndex(uSBaseIndedx){}
	~XTextureSet();

	void Release();
	virtual UINT GetSBaseIndex() { return m_uSBaseIndex; }
	UINT GetTextureCount() { return m_vpTexture.size(); }
};

struct STextureLayer
{
	std::wstring							m_sFileName;
	DXGI_FORMAT								m_Format;
	UINT									m_uWidth, m_uHeight, m_uPixelSize;
	UINT8									*m_pData;

	ComPtr<ID3D12Resource>					m_pTextureUpload;
};
typedef UINT8* (*CreateTextureFun)(UINT uWidth, UINT uHeight, UINT uPixelSize, UINT uParameter);
struct TextureSetLoad : public IResourceLoad
{
	UINT									m_uLayerCount;
	std::vector<STextureLayer>				m_vTextureLayer;

	XTextureSet								*m_pTextureSet;
	CreateTextureFun						m_pFun;
	UINT									m_uParameter;

	//
	~TextureSetLoad();

	virtual void LoadFromFile();
	virtual void PostLoad();
	virtual bool IsNeedWaitForResource() { return true; };
};