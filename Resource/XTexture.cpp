

#include "XTexture.h"
#include "DDSTextureLoader.h"
#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt);

extern XEngine *g_pEngine;
extern XResourceThread *g_pResourceThread;

extern ID3D12DescriptorHeap	*GetHandleHeap(XEngine::XDescriptorHeapType eType);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

//
XTextureSet::~XTextureSet()
{
	Release();
}
void XTextureSet::Release()
{
	for (UINT i = 0;i < m_vpTexture.size();++i)
	{
		if (m_vpTexture[i])
		{
			m_vpTexture[i]->Release();
		}
	}
	m_vpTexture.clear();
}

IWICImagingFactory* XTextureSet::m_pWIC;
std::map<std::wstring, XTextureSet*> XTextureSet::m_mTextureSet;
IWICImagingFactory* _GetWIC()
{
	return XTextureSet::GetImagingFactory();
}

void XTextureSet::Init(ID3D12Device* pDevice)
{
	//
	m_pWIC = nullptr;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory2,
		(LPVOID*)&m_pWIC);
}

void XTextureSet::Clean()
{
	//
	if (m_pWIC)
	{
		m_pWIC->Release();
	}
}

XTextureSet* XTextureSet::CreateTextureSet(LPCWSTR pName, UINT uCount, LPCWSTR pFileName[], eTextureType eType[],UINT uSRVIndex, eTextureFileType eFileType)
{
	XTextureSet *pTextureSet = nullptr;
	std::map<std::wstring, XTextureSet*>::iterator it = XTextureSet::m_mTextureSet.find(pName);
	if (it != XTextureSet::m_mTextureSet.end())
	{
		pTextureSet = it->second;
		if (pTextureSet)
		{
			pTextureSet->AddRef();
		}
		return pTextureSet;
	}

	//
	pTextureSet = new XTextureSet(pName,uSRVIndex);
	XTextureSet::m_mTextureSet[pName] = pTextureSet;

	//
	TextureSetLoad *pTextureSetLoad = nullptr;
	switch (eFileType)
	{
	case ETEXTUREFILETYPE_DDS:
		pTextureSetLoad = new DDSTextureSetLoad();
		break;
	case ETEXTUREFILETYPE_OTHER:
		pTextureSetLoad = new TextureSetLoad();
		break;
	}
	pTextureSetLoad->m_pTextureSet = pTextureSet;

	for (UINT i = 0;i < uCount;++i)
	{
		STextureLayer sTextureLayer;
		sTextureLayer.m_sFileName = pFileName[i];
		sTextureLayer.m_eType = eType[i];
		pTextureSetLoad->m_vTextureLayer.push_back(sTextureLayer);
	}

	g_pResourceThread->InsertResourceLoadTask(pTextureSetLoad);
	return pTextureSet;
}

XTextureSet* XTextureSet::CreateTextureSet(LPCWSTR pName, UINT uCount, LPCWSTR pFileName[], UINT uSRVIndex, eTextureFileType eFileType)
{
	eTextureType eType[8] = { ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D ,ETEXTURETYPE_2D };
	return XTextureSet::CreateTextureSet(pName, uCount, pFileName, eType, uSRVIndex, eFileType);
}

XTextureSet* XTextureSet::CreateCubeTexture(LPCWSTR pName, LPCWSTR pFileName, UINT uSRVIndex, eTextureFileType eFileType)
{
	eTextureType eType[8] = { ETEXTURETYPE_CUBE };
	return XTextureSet::CreateTextureSet(pName, 1, &pFileName, eType, uSRVIndex, eFileType);
}

XTextureSet* XTextureSet::CreateTextureSet(LPCWSTR pName, UINT uSRVIndex, UINT uWidth, UINT uHeight, DXGI_FORMAT Format, UINT8 *pData, UINT uPixelSize)
{
	XTextureSet *pTextureSet = nullptr;
	std::map<std::wstring, XTextureSet*>::iterator it = XTextureSet::m_mTextureSet.find(pName);
	if (it != XTextureSet::m_mTextureSet.end())
	{
		pTextureSet = it->second;
		if (pTextureSet)
		{
			pTextureSet->AddRef();
		}
		return pTextureSet;
	}

	//
	pTextureSet = new XTextureSet(pName, uSRVIndex);
	XTextureSet::m_mTextureSet[pName] = pTextureSet;

	//
	TextureSetLoad *pTextureSetLoad = new MemoryTextureSetLoad();
	pTextureSetLoad->m_pTextureSet = pTextureSet;

	STextureLayer sTextureLayer;
	sTextureLayer.m_Format = Format;
	sTextureLayer.m_uWidth	= uWidth;
	sTextureLayer.m_uHeight = uHeight;
	sTextureLayer.m_uPixelSize = uPixelSize;
	sTextureLayer.m_pData = pData;

	pTextureSetLoad->m_vTextureLayer.push_back(sTextureLayer);

	g_pResourceThread->InsertResourceLoadTask(pTextureSetLoad);
	return pTextureSet;
}

void XTextureSet::DeleteTextureSet(XTextureSet** ppTextureSet)
{
	if (*ppTextureSet)
	{
		int iRef = (*ppTextureSet)->DecRef();
		if (iRef <= 0)
		{
			std::map<std::wstring, XTextureSet*>::iterator it = XTextureSet::m_mTextureSet.find((*ppTextureSet)->GetName());
			if (it != XTextureSet::m_mTextureSet.end())
			{
				XTextureSet::m_mTextureSet.erase(it);
			}
			SAFE_DELETE(*ppTextureSet);
		}
		*ppTextureSet = nullptr;
	}
}

//
TextureSetLoad::~TextureSetLoad()
{
	for (UINT i = 0;i < m_vTextureLayer.size();++i)
	{
		if (m_vTextureLayer[i].m_pData)
		{
			delete[] m_vTextureLayer[i].m_pData;
		}
	}
	m_vTextureLayer.clear();
}

extern UINT8* CreateTextureFromWIC(LPCWSTR pFileName, DXGI_FORMAT& Format, UINT& PixelSize, UINT& Width, UINT& Height);
void TextureSetLoad::LoadFromFile()
{
	// Create an upload heap to load the texture onto the GPU. ComPtr's are CPU objects
	// but this heap needs to stay in scope until the GPU work is complete. We will
	// synchronize with the GPU at the end of this method before the ComPtr is destroyed.
	//if (m_pFileName)
	{
		for (UINT i = 0;i < m_vTextureLayer.size();++i)
		{
			m_vTextureLayer[i].m_pData = CreateTextureFromWIC(m_vTextureLayer[i].m_sFileName.c_str(), m_vTextureLayer[i].m_Format, m_vTextureLayer[i].m_uPixelSize, m_vTextureLayer[i].m_uWidth, m_vTextureLayer[i].m_uHeight);
		}
	}
/*
	else
	{
		if (m_pFun)
		{
			m_vTextureLayer[0].m_pData = (UINT8*)((*m_pFun)(m_vTextureLayer[0].m_uWidth, m_vTextureLayer[0].m_uHeight, m_vTextureLayer[0].m_uPixelSize, m_uParameter));
			m_vTextureLayer[0].m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
*/
}
void TextureSetLoad::PostLoad()
{
	//
	for (UINT i = 0;i < m_vTextureLayer.size();++i)
	{
		// Create the texture.
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = m_vTextureLayer[i].m_Format;//DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = m_vTextureLayer[i].m_uWidth;
		textureDesc.Height = m_vTextureLayer[i].m_uHeight;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ID3D12Resource *pTexture = nullptr;
		ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pTexture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexture, 0, 1);

		// Create the GPU upload buffer.
		ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vTextureLayer[i].m_pTextureUpload)));

		//
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = m_vTextureLayer[i].m_pData;
		textureData.RowPitch = m_vTextureLayer[i].m_uWidth * m_vTextureLayer[i].m_uPixelSize;
		textureData.SlicePitch = textureData.RowPitch * m_vTextureLayer[i].m_uHeight;

		UpdateSubresources(g_pResourceThread->GetResourceCommandList(), pTexture, m_vTextureLayer[i].m_pTextureUpload.Get(), 0, 0, 1, &textureData);
		g_pResourceThread->GetResourceCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		switch (m_vTextureLayer[i].m_eType)
		{
		case XTextureSet::ETEXTURETYPE_CUBE:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
			break;
		}
		
		//
		g_pEngine->m_pDevice->CreateShaderResourceView(pTexture, &srvDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex() + i));

		//
		m_pTextureSet->m_vpTexture.push_back(pTexture);
	}
	m_pTextureSet->m_hSRVCpuHandle = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex());
	m_pTextureSet->m_hSRVGpuHandle = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex());

	//m_pResourceSet->IncreaseResourceComplate();
}

XRenderTarget* XRenderTarget::CreateRenderTarget(DXGI_FORMAT Format,UINT uWidth,UINT uHeight,UINT uRTVIndex,UINT uSRVIndex,UINT uUAVIndex)
{
	XRenderTarget *pRenderTarget = new XRenderTarget;

	D3D12_RESOURCE_DESC textureDesc = {};

	textureDesc.MipLevels = 1;
	textureDesc.Format = Format;
	textureDesc.Width = uWidth;
	textureDesc.Height = uHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET| D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	D3D12_CLEAR_VALUE rendertargetOptimizedClearValue = {};
	rendertargetOptimizedClearValue.Format = textureDesc.Format;
	rendertargetOptimizedClearValue.Color[0] = 0.0f;
	rendertargetOptimizedClearValue.Color[1] = 0.0f;
	rendertargetOptimizedClearValue.Color[2] = 0.0f;
	rendertargetOptimizedClearValue.Color[3] = 0.0f;

	//
	ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&rendertargetOptimizedClearValue,
		IID_PPV_ARGS(&pRenderTarget->m_pRenderTarget)));

	//
	pRenderTarget->m_hRTVCpuHandle = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_RTV, uRTVIndex);
	pRenderTarget->m_hRTVGpuHandle = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_RTV, uRTVIndex);
	g_pEngine->m_pDevice->CreateRenderTargetView(pRenderTarget->m_pRenderTarget.Get(), nullptr, pRenderTarget->m_hRTVCpuHandle);

	//
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	pRenderTarget->m_hSRVCpuHandle = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uSRVIndex);
	pRenderTarget->m_hSRVGpuHandle = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uSRVIndex);
	g_pEngine->m_pDevice->CreateShaderResourceView(pRenderTarget->m_pRenderTarget.Get(), &srvDesc, pRenderTarget->m_hSRVCpuHandle);

	//
	if (uUAVIndex != 0xFFFFFFFF)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = Format;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UDesc.Texture2D.MipSlice = 0;
		UDesc.Texture2D.PlaneSlice = 0;

		pRenderTarget->m_hUAVCpuHandle = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uUAVIndex);
		pRenderTarget->m_hUAVGpuHandle = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uUAVIndex);
		g_pEngine->m_pDevice->CreateUnorderedAccessView(pRenderTarget->m_pRenderTarget.Get(), nullptr, &UDesc, pRenderTarget->m_hUAVCpuHandle);
	}

	return pRenderTarget;
}

extern DXGI_FORMAT ddsFormat;
void DDSTextureSetLoad::LoadFromFile()
{
	for (UINT i = 0;i < m_vTextureLayer.size();++i)
	{
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		ID3D12Resource *pTexture = nullptr;
		LoadDDSTextureFromFileEx(g_pEngine->m_pDevice, m_vTextureLayer[i].m_sFileName.c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &pTexture, ddsData, subresources);

		//
		UINT uCount = (UINT)subresources.size();
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexture, 0, uCount);

		// Create the GPU upload buffer.
		ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vTextureLayer[i].m_pTextureUpload)));

		//
		UpdateSubresources(g_pResourceThread->GetResourceCommandList(), pTexture, m_vTextureLayer[i].m_pTextureUpload.Get(), 0, 0, uCount, &subresources[0]);
		g_pResourceThread->GetResourceCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = ddsFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		switch (m_vTextureLayer[i].m_eType)
		{
		case XTextureSet::ETEXTURETYPE_CUBE:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
			break;
		}

		//
		//TextureManager *pTextureManager = GetXEngine()->GetTextureManager();
		
		g_pEngine->m_pDevice->CreateShaderResourceView(pTexture, &srvDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex() + i));

		//
		m_pTextureSet->m_vpTexture.push_back(pTexture);
	}
	m_pTextureSet->m_hSRVCpuHandle = GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex());
	m_pTextureSet->m_hSRVGpuHandle = GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pTextureSet->GetSBaseIndex());
}

void DDSTextureSetLoad::PostLoad()
{
}

void MemoryTextureSetLoad::LoadFromFile()
{
}

void MemoryTextureSetLoad::PostLoad()
{
	TextureSetLoad::PostLoad();
}

///////////////////////////////////////////////////////
struct WICConvert
{
	GUID        source;
	GUID        target;
};
static WICConvert g_WICConvert[] =
{
	// Note target GUID in this conversion table must be one of those directly supported formats (above).

	{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

	{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

	{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
	{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

	{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
	{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

	{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

	{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

	{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

	{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

	{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

	{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

	{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
#endif

																				// We don't support n-channel formats
};
struct WICTranslate
{
	GUID                wic;
	DXGI_FORMAT         format;
};
static WICTranslate g_WICFormats[] =
{
	{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

	{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
	{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

	{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
	{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

	{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

	{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
	{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

	{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
	{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
	{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
	{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

	{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },
};
static DXGI_FORMAT _WICToDXGI(const GUID& guid)
{
	for (size_t i = 0; i < _countof(g_WICFormats); ++i)
	{
		if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
			return g_WICFormats[i].format;
	}

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	if (true)
	{
		if (memcmp(&GUID_WICPixelFormat96bppRGBFloat, &guid, sizeof(GUID)) == 0)
			return DXGI_FORMAT_R32G32B32_FLOAT;
	}
#endif

	return DXGI_FORMAT_UNKNOWN;
}
static size_t _WICBitsPerPixel(REFGUID targetGuid)
{
	IWICImagingFactory* pWIC = _GetWIC();
	if (!pWIC)
		return 0;

	ComPtr<IWICComponentInfo> cinfo;
	if (FAILED(pWIC->CreateComponentInfo(targetGuid, cinfo.GetAddressOf())))
		return 0;

	WICComponentType type;
	if (FAILED(cinfo->GetComponentType(&type)))
		return 0;

	if (type != WICPixelFormat)
		return 0;

	ComPtr<IWICPixelFormatInfo> pfinfo;
	if (FAILED(cinfo.As(&pfinfo)))
		return 0;

	UINT bpp;
	if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
		return 0;

	return bpp;
}
UINT8* CreateTextureFromWIC(LPCWSTR pFileName, DXGI_FORMAT& Format, UINT& PixelSize, UINT& Width, UINT& Height)
{
	IWICImagingFactory* pWIC = _GetWIC();
	if (!pWIC)
	{
		return nullptr;
	}

	//
	bool forceSRGB = false;

	//
	ComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = pWIC->CreateDecoderFromFilename(pFileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr))
	{
		return nullptr;
	}

	hr = frame->GetSize(&Width, &Height);
	if (FAILED(hr))
	{
		return nullptr;
	}

	assert(Width > 0 && Height > 0);

	size_t maxsize = 8192;
	assert(maxsize > 0);

	UINT twidth, theight;
	if (Width > maxsize || Height > maxsize)
	{
		float ar = static_cast<float>(Height) / static_cast<float>(Width);
		if (Width > Height)
		{
			twidth = static_cast<UINT>(maxsize);
			theight = static_cast<UINT>(static_cast<float>(maxsize) * ar);
		}
		else
		{
			theight = static_cast<UINT>(maxsize);
			twidth = static_cast<UINT>(static_cast<float>(maxsize) / ar);
		}
		assert(twidth <= maxsize && theight <= maxsize);
	}
	else
	{
		twidth = Width;
		theight = Height;
	}

	// Determine format
	WICPixelFormatGUID pixelFormat;
	hr = frame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr))
	{
		return nullptr;
	}

	WICPixelFormatGUID convertGUID;
	memcpy(&convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID));

	size_t bpp = 0;

	Format = _WICToDXGI(pixelFormat);
	if (Format == DXGI_FORMAT_UNKNOWN)
	{
		if (memcmp(&GUID_WICPixelFormat96bppRGBFixedPoint, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
		{
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
			if (true)
			{
				memcpy(&convertGUID, &GUID_WICPixelFormat96bppRGBFloat, sizeof(WICPixelFormatGUID));
				Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else
#endif
			{
				memcpy(&convertGUID, &GUID_WICPixelFormat128bppRGBAFloat, sizeof(WICPixelFormatGUID));
				Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}
		else
		{
			for (size_t i = 0; i < _countof(g_WICConvert); ++i)
			{
				if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
				{
					memcpy(&convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID));

					Format = _WICToDXGI(g_WICConvert[i].target);
					assert(Format != DXGI_FORMAT_UNKNOWN);
					bpp = _WICBitsPerPixel(convertGUID);
					break;
				}
			}
		}

		if (Format == DXGI_FORMAT_UNKNOWN)
		{
			//return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			return nullptr;
		}
	}
	else
	{
		bpp = _WICBitsPerPixel(pixelFormat);
	}

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	if ((Format == DXGI_FORMAT_R32G32B32_FLOAT))// && d3dContext != 0 && textureView != 0)
	{
		// Special case test for optional device support for autogen mipchains for R32G32B32_FLOAT 
		UINT fmtSupport = 0;
		//hr = d3dDevice->CheckFormatSupport(DXGI_FORMAT_R32G32B32_FLOAT, &fmtSupport);
		//if (FAILED(hr) || !(fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
		{
			// Use R32G32B32A32_FLOAT instead which is required for Feature Level 10.0 and up
			memcpy(&convertGUID, &GUID_WICPixelFormat128bppRGBAFloat, sizeof(WICPixelFormatGUID));
			Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			bpp = 128;
		}
	}
#endif

	if (!bpp)
	{
		return nullptr;
	}
	PixelSize = (UINT)(bpp / 8);

	// Handle sRGB formats
	forceSRGB = false;
	if (forceSRGB)
	{
		//format = MakeSRGB(format);
	}
	else
	{
		ComPtr<IWICMetadataQueryReader> metareader;
		if (SUCCEEDED(frame->GetMetadataQueryReader(metareader.GetAddressOf())))
		{
			GUID containerFormat;
			if (SUCCEEDED(metareader->GetContainerFormat(&containerFormat)))
			{
				// Check for sRGB colorspace metadata
				bool sRGB = false;

				PROPVARIANT value;
				PropVariantInit(&value);

				if (memcmp(&containerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) == 0)
				{
					// Check for sRGB chunk
					if (SUCCEEDED(metareader->GetMetadataByName(L"/sRGB/RenderingIntent", &value)) && value.vt == VT_UI1)
					{
						sRGB = true;
					}
				}
				else if (SUCCEEDED(metareader->GetMetadataByName(L"System.Image.ColorSpace", &value)) && value.vt == VT_UI2 && value.uiVal == 1)
				{
					sRGB = true;
				}

				PropVariantClear(&value);

				//if (sRGB)
				//	format = MakeSRGB(format);
			}
		}
	}

	// Verify our target format is supported by the current device
	// (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as DirectX 11.0 Runtime without 16bpp format support)
	/*
	UINT support = 0;
	hr = d3dDevice->CheckFormatSupport(format, &support);
	if (FAILED(hr) || !(support & D3D11_FORMAT_SUPPORT_TEXTURE2D))
	{
	// Fallback to RGBA 32-bit format which is supported by all devices
	memcpy(&convertGUID, &GUID_WICPixelFormat32bppRGBA, sizeof(WICPixelFormatGUID));
	Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bpp = 32;
	}
	*/
	// Allocate temporary memory for image
	size_t rowPitch = (twidth * bpp + 7) / 8;
	size_t imageSize = rowPitch * theight;

	//std::unique_ptr<uint8_t[]> temp(new (std::nothrow) uint8_t[imageSize]);
	UINT8 *pData = new UINT8[imageSize];
	if (!pData)
	{
		return nullptr;
	}

	// Load image data
	if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
		&& twidth == Width
		&& theight == Height)
	{
		// No format conversion or resize needed
		hr = frame->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), (BYTE*)pData);
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}
	}
	else if (twidth != Width || theight != Height)
	{
		// Resize
		IWICImagingFactory* pWIC = _GetWIC();
		if (!pWIC)
		{
			delete[] pData;
			return nullptr;
		}

		ComPtr<IWICBitmapScaler> scaler;
		hr = pWIC->CreateBitmapScaler(scaler.GetAddressOf());
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}

		hr = scaler->Initialize(frame.Get(), twidth, theight, WICBitmapInterpolationModeFant);
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}

		WICPixelFormatGUID pfScaler;
		hr = scaler->GetPixelFormat(&pfScaler);
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}

		if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
		{
			// No format conversion needed
			hr = scaler->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), (BYTE*)pData);
			if (FAILED(hr))
			{
				delete[] pData;
				return nullptr;
			}
		}
		else
		{
			ComPtr<IWICFormatConverter> FC;
			hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
			if (FAILED(hr))
			{
				delete[] pData;
				return nullptr;
			}

			BOOL canConvert = FALSE;
			hr = FC->CanConvert(pfScaler, convertGUID, &canConvert);
			if (FAILED(hr) || !canConvert)
			{
				//return E_UNEXPECTED;
				delete[] pData;
				return nullptr;
			}

			hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
			if (FAILED(hr))
			{
				delete[] pData;
				return nullptr;
			}

			hr = FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), (BYTE*)pData);
			if (FAILED(hr))
			{
				delete[] pData;
				return nullptr;
			}
		}
	}
	else
	{
		// Format conversion but no resize
		IWICImagingFactory* pWIC = _GetWIC();
		if (!pWIC)
		{
			//return E_NOINTERFACE;
			delete[] pData;
			return nullptr;
		}

		ComPtr<IWICFormatConverter> FC;
		hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}

		BOOL canConvert = FALSE;
		hr = FC->CanConvert(pixelFormat, convertGUID, &canConvert);
		if (FAILED(hr) || !canConvert)
		{
			//return E_UNEXPECTED;
			delete[] pData;
			return nullptr;
		}

		hr = FC->Initialize(frame.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}

		hr = FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), (BYTE*)pData);
		if (FAILED(hr))
		{
			delete[] pData;
			return nullptr;
		}
	}

	// See if format is supported for auto-gen mipmaps (varies by feature level)
	bool autogen = true;//false;
						/*
						if (d3dContext != 0 && textureView != 0) // Must have context and shader-view to auto generate mipmaps
						{
						UINT fmtSupport = 0;
						hr = d3dDevice->CheckFormatSupport(format, &fmtSupport);
						if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
						{
						autogen = true;
						}
						}

						// Create texture
						D3D11_TEXTURE2D_DESC desc;
						desc.Width = twidth;
						desc.Height = theight;
						desc.MipLevels = (autogen) ? 0 : 1;
						desc.ArraySize = 1;
						desc.Format = format;
						desc.SampleDesc.Count = 1;
						desc.SampleDesc.Quality = 0;
						desc.Usage = usage;
						desc.CPUAccessFlags = cpuAccessFlags;

						if (autogen)
						{
						desc.BindFlags = bindFlags | D3D11_BIND_RENDER_TARGET;
						desc.MiscFlags = miscFlags | D3D11_RESOURCE_MISC_GENERATE_MIPS;
						}
						else
						{
						desc.BindFlags = bindFlags;
						desc.MiscFlags = miscFlags;
						}

						D3D11_SUBRESOURCE_DATA initData;
						initData.pSysMem = temp.get();
						initData.SysMemPitch = static_cast<UINT>(rowPitch);
						initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

						ID3D11Texture2D* tex = nullptr;
						hr = d3dDevice->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &tex);
						if (SUCCEEDED(hr) && tex != 0)
						{
						if (textureView != 0)
						{
						D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
						memset(&SRVDesc, 0, sizeof(SRVDesc));
						SRVDesc.Format = desc.Format;

						SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

						hr = d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);
						if (FAILED(hr))
						{
						tex->Release();
						return hr;
						}

						if (autogen)
						{
						assert(d3dContext != 0);
						d3dContext->UpdateSubresource(tex, 0, nullptr, temp.get(), static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize));
						d3dContext->GenerateMips(*textureView);
						}
						}

						if (texture != 0)
						{
						*texture = tex;
						}
						else
						{
						SetDebugObjectName(tex, "WICTextureLoader");
						tex->Release();
						}
						}
						*/
	return pData;
}

//--------------------------------------------------------------------------------------
// Return the BPP for a particular format
//--------------------------------------------------------------------------------------
inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

	case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
	case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
	case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
		return 32;

	case DXGI_FORMAT_D16_UNORM_S8_UINT:
	case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		return 24;

	case DXGI_FORMAT_R4G4_UNORM:
		return 8;

#endif // _XBOX_ONE && _TITLE

	default:
		return 0;
	}
}