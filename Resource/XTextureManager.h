
#pragma once

#include "XTexture.h"

#include <Wincodec.h>
#include <DirectXMath.h>
using namespace DirectX;
using namespace Microsoft::WRL;

//
#define TEXTURELAYER_MAX		8
class XTextureManager
{
	//ComPtr<ID3D12DescriptorHeap>			m_pSrvHeap;
	//std::list<UINT>						m_lUsedSrv[TEXTURELAYER_MAX], m_lFreeSrv[TEXTURELAYER_MAX];
	//UINT									m_uSrvDescriptorSize;
	ComPtr<IWICImagingFactory>				m_pWIC;

public:
	~XTextureManager();

	void Init(ID3D12Device* pDevice);
	//ID3D12DescriptorHeap* GetDescriptorHeap();
	//UINT GetFreeSrv(UINT uLayerCount = 1);
	//void AddFreeSrv(UINT uSrvIndex, UINT uLayerCount = 1);

	//CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHangle(UINT uIndex);
	//CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHangle(UINT uIndex);
	IWICImagingFactory *GetImagingFactory() { return m_pWIC.Get(); }

	//
	//ITexture* CreateTexture(LPCWSTR pTextureName, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter,ResourceSet* pResourceSet);
	//ITexture* CreateTextureFromFile(LPCWSTR pFileName, UINT uCount, LPCWSTR pDetailName[], ResourceSet* pResourceSet);
	//void DeleteTexture(ITexture*& pTexture);
};