
#include "XBinLoader.h"
#include "occcity.h"

void XBinResource::LoadFromFile()
{
	ReadDataFromFile(L"occcity.bin", &pMeshData, &meshDataLength);
	pEntity->InitGeometry(L"occcity", SampleAssets::VertexDataSize / SampleAssets::StandardVertexStride, SampleAssets::StandardVertexStride, SampleAssets::IndexDataSize / 4, DXGI_FORMAT_R32_UINT, pMeshData + SampleAssets::VertexDataOffset);
	pEntity->InitTexture(L"occcity", SampleAssets::Textures[0].Width, SampleAssets::Textures[0].Height, SampleAssets::Textures[0].Format, pMeshData + SampleAssets::Textures[0].Data[0].Offset, SampleAssets::Textures[0].Data[0].Pitch / SampleAssets::Textures[0].Width);
}