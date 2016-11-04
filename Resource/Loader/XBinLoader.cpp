
#include "XBinLoader.h"
#include "occcity.h"

void XBinResource::LoadFromFile()
{
	XGeometry *pGeometry = XGeometryManager::GetResource(m_pFileName);
	XTextureSet *pTexture = XTextureSetManager::GetResource(m_pFileName);

	if ((!pGeometry)||(!pTexture))
	{
		ReadDataFromFile(m_pFileName, &m_pMeshData, &m_meshDataLength);
	}

	if (!pGeometry)
	{
		m_pEntity->InitGeometry(m_pFileName, SampleAssets::VertexDataSize / SampleAssets::StandardVertexStride, SampleAssets::StandardVertexStride, SampleAssets::IndexDataSize / 4, DXGI_FORMAT_R32_UINT, m_pMeshData + SampleAssets::VertexDataOffset);
	}
	else
	{
		m_pEntity->m_pGeometry = pGeometry;
	}
	if (!pTexture)
	{
		m_pEntity->InitTexture(m_pFileName, SampleAssets::Textures[0].Width, SampleAssets::Textures[0].Height, SampleAssets::Textures[0].Format, m_pMeshData + SampleAssets::Textures[0].Data[0].Offset, SampleAssets::Textures[0].Data[0].Pitch / SampleAssets::Textures[0].Width);
	}
	else
	{
		m_pEntity->m_pTextureSet = pTexture;
	}
}