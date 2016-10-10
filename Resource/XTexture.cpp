

#include "XTexture.h"
#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

extern ComPtr<ID3D12Device>				g_pDevice;
extern XResourceThread					g_ResourceThread;

extern ComPtr<ID3D12DescriptorHeap>		g_pCSUDescriptorHeap;
extern UINT								g_uCSUDescriptorSize;

//
XTextureSet::~XTextureSet()
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
void XTextureSet::Release()
{
	//UINT8 uLayerIndex = m_vpTexture.size();
	for (UINT i = 0;i < m_vpTexture.size();++i)
	{
		if (m_vpTexture[i])
		{
			m_vpTexture[i]->Release();
		}
	}
	m_vpTexture.clear();
	//GetXEngine()->GetTextureManager()->AddFreeIndex(m_uSrvIndex, uLayerIndex);
}

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

//
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
		ThrowIfFailed(g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pTexture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexture, 0, 1);

		// Create the GPU upload buffer.
		ThrowIfFailed(g_pDevice->CreateCommittedResource(
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

		UpdateSubresources(g_ResourceThread.GetResourceCommandList(), pTexture, m_vTextureLayer[i].m_pTextureUpload.Get(), 0, 0, 1, &textureData);
		g_ResourceThread.GetResourceCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		//
		//TextureManager *pTextureManager = GetXEngine()->GetTextureManager();
		g_pDevice->CreateShaderResourceView(pTexture, &srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCSUDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_pTextureSet->GetSBaseIndex() + i, g_uCSUDescriptorSize));

		//
		m_pTextureSet->m_vpTexture.push_back(pTexture);
	}

	//m_pResourceSet->IncreaseResourceComplate();
}
