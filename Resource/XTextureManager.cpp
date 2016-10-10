
#include "XTextureManager.h"

#include <memory>
#include "..\DXSampleHelper.h"

XTextureManager				*g_pTextureManager = nullptr;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IWICImagingFactory* _GetWIC()
{
	return g_pTextureManager->GetImagingFactory();
}

XTextureManager::~XTextureManager()
{
}

void XTextureManager::Init(ID3D12Device* pDevice)
{
	// Describe and create a shader resource view (SRV) heap for the texture.
/*
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

	UINT uCount = 0;
	for (UINT i = 0;i < TEXTURELAYER_MAX;++i)
	{
		uCount += TextureCount[i] * (i+1);
	}
	srvHeapDesc.NumDescriptors = uCount;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSrvHeap)));

	//
	UINT uIndex = 0;
	for (UINT i = 0;i < TEXTURELAYER_MAX;++i)
	{
		m_lFreeSrv[i].push_back(uIndex);
		uIndex += TextureCount[i] * (i+1);
	}

	//
	m_uSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
*/
	//UINT uCountPerLayer[] = { 512,512,512,512,512,512,512,512 };
	//ResourceHeap<TEXTURELAYER_MAX>::Init(pDevice, uCountPerLayer);

	//
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory2,
		(LPVOID*)&m_pWIC);
}

/*
ID3D12DescriptorHeap* TextureManager::GetDescriptorHeap()
{
	return m_pSrvHeap.Get();
}

UINT TextureManager::GetFreeSrv(UINT uLayerCount)
{
	if ((uLayerCount > TEXTURELAYER_MAX)||(uLayerCount==0))
	{
		return 0xFFFFFFFF;
	}
	if (m_lFreeSrv[uLayerCount-1].size() == 0)
	{
		return 0xFFFFFFFF;
	}
	UINT uIndex = *(m_lFreeSrv[uLayerCount-1].begin());
	m_lFreeSrv[uLayerCount-1].pop_front();

	return uIndex;
}
void TextureManager::AddFreeSrv(UINT uSrvIndex, UINT uLayerCount)
{
	if ((uLayerCount > TEXTURELAYER_MAX) || (uLayerCount == 0))
	{
		return;
	}
	std::list<UINT>::const_iterator it = find(m_lFreeSrv[uLayerCount - 1].begin(), m_lFreeSrv[uLayerCount - 1].end(), uSrvIndex);
	if (it == m_lFreeSrv[uLayerCount-1].end())
	{
		m_lFreeSrv[uLayerCount - 1].push_back(uSrvIndex);
	}
}

CD3DX12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGpuHangle(UINT uIndex)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pSrvHeap->GetGPUDescriptorHandleForHeapStart(), uIndex, m_uSrvDescriptorSize);
}
CD3DX12_CPU_DESCRIPTOR_HANDLE TextureManager::GetCpuHangle(UINT uIndex)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pSrvHeap->GetCPUDescriptorHandleForHeapStart(), uIndex, m_uSrvDescriptorSize);
}
*/
//
/*
ITexture* TextureManager::CreateTexture(LPCWSTR pTextureName, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter, ResourceSet* pResourceSet)
{
	//
	Texture *pTexture = GetResource(pTextureName);
	if (pTexture)
	{
		pTexture->AddRefCount();
		pResourceSet->IncreaseResourceComplate();
		return pTexture;
	}

	UINT uIndex = GetFreeIndex();
	if (uIndex == 0xFFFFFFFF)
	{
		return nullptr;
	}

	//
	pTexture = new Texture(uIndex);
	AddResource(pTextureName, pTexture);

	GetXEngine()->GetResourceThread()->InsertTextureLoadTask(nullptr, pTexture, uWidth, uHeight, uPixelSize, pFun, uParameter, pResourceSet);
	return pTexture;
}
ITexture* TextureManager::CreateTextureFromFile(LPCWSTR pFileName, UINT uCount, LPCWSTR pDetailName[], ResourceSet* pResourceSet)
{
	//
	Texture *pTexture = GetResource(pFileName);
	if (pTexture)
	{
		pTexture->AddRefCount();
		pResourceSet->IncreaseResourceComplate();
		return pTexture;
	}

	UINT uIndex = GetFreeIndex(uCount-1);
	if (uIndex == 0xFFFFFFFF)
	{
		return nullptr;
	}

	//
	pTexture = new Texture(uIndex);
	AddResource(pFileName, pTexture);

	GetXEngine()->GetResourceThread()->InsertTextureLoadTask(pFileName, pTexture, uCount, pDetailName, pResourceSet);
	return pTexture;
}

void TextureManager::DeleteTexture(ITexture*& pITexture)
{
	ResourceManager::DelResource(pITexture);
}
*/
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
	PixelSize = bpp / 8;

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