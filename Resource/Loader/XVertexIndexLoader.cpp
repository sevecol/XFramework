
#include "XVertexIndexLoader.h"

#include <stdlib.h>    
using namespace std;

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 textureCoordinate;
	DirectX::XMFLOAT3 tangent;
};

bool LoadVertices(const WCHAR* FileName, std::vector<Vertex>& OutVertices)
{
	FILE* fp = _wfopen(FileName, L"rb");
	if (!fp) return false;

	fseek(fp, 0L, SEEK_END);
	UINT FileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	UINT NumVertices = FileSize / sizeof(Vertex::position);

	for (UINT Idx = 0; Idx < NumVertices; ++Idx)
	{
		Vertex vertex;
		fread(&vertex.position, sizeof(XMFLOAT3), 1, fp);
		vertex.normal.x = vertex.normal.y = 0;
		vertex.normal.z = 1.0;
		vertex.textureCoordinate.x = vertex.textureCoordinate.y = 0.5f;
		vertex.tangent.x = vertex.tangent.y = vertex.tangent.z = 0.0f;
		OutVertices.push_back(vertex);
	}

	fclose(fp);
	return true;
}

bool LoadIndices(const WCHAR* FileName, std::vector<uint32_t>& OutIndices)
{
	FILE* fp = _wfopen(FileName, L"rb");
	if (!fp) return false;

	fseek(fp, 0L, SEEK_END);
	UINT FileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	UINT NumIndices = FileSize / sizeof(uint32_t);

	for (UINT Idx = 0; Idx < NumIndices; ++Idx)
	{
		uint32_t index;
		fread(&index, sizeof(index), 1, fp);
		OutIndices.push_back(index);
	}

	fclose(fp);
	return true;
}
extern UINT8 GeometryData[20480000];
void XVertexIndexResource::LoadFromFile()
{
	XGeometry *pGeometry = XGeometryManager::GetResource(L"entityvi");
	if (!pGeometry)
	{
		vector<Vertex> vVertex;
		vector<uint32_t> vIndex;

		LoadVertices(L"Media\\SibenikVertices.bin", vVertex);
		LoadIndices(L"Media\\SibenikIndices.bin", vIndex);

		UINT uSize = vVertex.size()*sizeof(Vertex) + vIndex.size()*sizeof(UINT);

		UINT uOffset = 0;
		memcpy(GeometryData + uOffset, &(vVertex[0]), vVertex.size()*sizeof(Vertex));
		uOffset = vVertex.size()*sizeof(Vertex);
		memcpy(GeometryData + uOffset, &(vIndex[0]), vIndex.size()*sizeof(UINT));

		pEntity->InitGeometry(L"entityvi", vVertex.size(), sizeof(Vertex), vIndex.size(), DXGI_FORMAT_R32_UINT, GeometryData);

		//
		//delete[] pGeometryData;

		return;
	}

	//
	pEntity->m_pGeometry = pGeometry;
}