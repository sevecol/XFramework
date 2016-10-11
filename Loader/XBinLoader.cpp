
#include "XBinLoader.h"
#include "occcity.h"

void XBinResource::LoadFromFile()
{
	ReadDataFromFile(L"occcity.bin", &pMeshData, &meshDataLength);
	pEntity->InitGeometry(L"111", SampleAssets::VertexDataSize / SampleAssets::StandardVertexStride, SampleAssets::StandardVertexStride, SampleAssets::IndexDataSize / 4, DXGI_FORMAT_R32_UINT, pMeshData + SampleAssets::VertexDataOffset);
}