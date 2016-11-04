
#include "XObjLoader.h"

#include <iostream>  
#include <fstream>  
#include <stdlib.h>    
#include <sstream>  
#include <string> 	
using namespace std;

struct sPosition
{
	float	fValue[3];
};
struct sTexcoord
{
	float	fValue[2];
};
struct sVertex
{
	sPosition vPosition, vNormal;
	sTexcoord vTexcoord;
	sPosition vTangent;
};

#define EPSILON 0.00000001
void ComputeTangent(sPosition& p0, sPosition& p1, sPosition& p2, sTexcoord& t0, sTexcoord& t1, sTexcoord& t2, sPosition& tangent)
{
	XMFLOAT3	p20 = XMFLOAT3(p1.fValue[0] - p0.fValue[0], p1.fValue[1] - p0.fValue[1], p1.fValue[2] - p0.fValue[2]);
	XMFLOAT3	p30 = XMFLOAT3(p2.fValue[0] - p0.fValue[0], p2.fValue[1] - p0.fValue[1], p2.fValue[2] - p0.fValue[2]);

	float		u20 = t1.fValue[0] - t0.fValue[0];
	float		u30 = t2.fValue[0] - t0.fValue[0];
	float		v20 = t1.fValue[1] - t0.fValue[1];
	float		v30 = t2.fValue[1] - t0.fValue[1];

	//
	tangent.fValue[0] = (p30.x - (u30 / v20 + EPSILON)*p20.x) / (u30 - (u30 / v20 + EPSILON)*u20);
	tangent.fValue[1] = (p30.y - (u30 / v20 + EPSILON)*p20.y) / (u30 - (u30 / v20 + EPSILON)*u20);
	tangent.fValue[2] = (p30.z - (u30 / v20 + EPSILON)*p20.z) / (u30 - (u30 / v20 + EPSILON)*u20);

	float fLength = sqrt(tangent.fValue[0] * tangent.fValue[0] + tangent.fValue[1] * tangent.fValue[1] + tangent.fValue[2] * tangent.fValue[2]);
	tangent.fValue[0] /= fLength + EPSILON;
	tangent.fValue[1] /= fLength + EPSILON;
	tangent.fValue[2] /= fLength + EPSILON;
}

void ReadDataFromObjFile(LPCWSTR filename, vector<sVertex>& vVertex, vector<UINT>& vIndex)
{
	ifstream inf;
	inf.open(filename);

	vector<sPosition> vPosition;
	vector<sPosition> vNormal;
	vector<sTexcoord> vTexcoord;

	struct sVertexTable
	{
		vector<UINT>	vVertexIndex;
		sPosition	 	Tangent;
	};
	map<UINT, sVertexTable> mVertexTable;

	//
	string sLine;
	string s1, s2, s3, s4;

	UINT uVertexCount = 0;
	UINT uFaceCount = 0;
	while (getline(inf, sLine))
	{
		istringstream in(sLine);
		in >> s1 >> s2 >> s3 >> s4;

		if (!s1.compare("v"))
		{
			sPosition position;
			position.fValue[0] = atof(s2.c_str());
			position.fValue[1] = atof(s3.c_str());
			position.fValue[2] = atof(s4.c_str());

			vPosition.push_back(position);
		}
		if (!s1.compare("vt"))
		{
			sTexcoord texcoord;
			texcoord.fValue[0] = atof(s2.c_str());
			texcoord.fValue[1] = atof(s3.c_str());

			vTexcoord.push_back(texcoord);
		}
		if (!s1.compare("vn"))
		{
			sPosition normal;
			normal.fValue[0] = atof(s2.c_str());
			normal.fValue[1] = atof(s3.c_str());
			normal.fValue[2] = atof(s4.c_str());

			vNormal.push_back(normal);
		}
		if (!s1.compare("f"))
		{
			UINT uIndex[3][3];
			sscanf(s2.c_str(), "%d/%d/%d", &uIndex[0][0], &uIndex[0][1], &uIndex[0][2]); 
			sscanf(s3.c_str(), "%d/%d/%d", &uIndex[1][0], &uIndex[1][1], &uIndex[1][2]);
			sscanf(s4.c_str(), "%d/%d/%d", &uIndex[2][0], &uIndex[2][1], &uIndex[2][2]);

			sVertex vertex1;
			vertex1.vPosition = vPosition[uIndex[0][0] - 1];
			vertex1.vTexcoord = vTexcoord[uIndex[0][1] - 1];
			vertex1.vNormal = vNormal[uIndex[0][2] - 1];
			vVertex.push_back(vertex1);
			vIndex.push_back(uVertexCount++);

			sVertex vertex2;
			vertex2.vPosition = vPosition[uIndex[1][0] - 1];
			vertex2.vTexcoord = vTexcoord[uIndex[1][1] - 1];
			vertex2.vNormal = vNormal[uIndex[1][2] - 1];
			vVertex.push_back(vertex2);
			vIndex.push_back(uVertexCount++);

			sVertex vertex3;
			vertex3.vPosition = vPosition[uIndex[2][0] - 1];
			vertex3.vTexcoord = vTexcoord[uIndex[2][1] - 1];
			vertex3.vNormal = vNormal[uIndex[2][2] - 1];
			vVertex.push_back(vertex3);
			vIndex.push_back(uVertexCount++);

			//
			sPosition tangent;
			ComputeTangent(vertex1.vPosition, vertex2.vPosition, vertex3.vPosition, vertex1.vTexcoord, vertex2.vTexcoord, vertex3.vTexcoord, tangent);

			//
			for (UINT i = 0;i < 3;++i)
			{
				UINT uValue = (uIndex[i][0] << 16) | uIndex[i][1];
				map<UINT, sVertexTable>::iterator it = mVertexTable.find(uValue);
				if (it != mVertexTable.end())
				{
					it->second.Tangent.fValue[0] += tangent.fValue[0];
					it->second.Tangent.fValue[1] += tangent.fValue[1];
					it->second.Tangent.fValue[2] += tangent.fValue[2];

					it->second.vVertexIndex.push_back(uFaceCount * 3 + i);
				}
				else
				{
					sVertexTable vertextable;
					vertextable.Tangent.fValue[0] = tangent.fValue[0];
					vertextable.Tangent.fValue[1] = tangent.fValue[1];
					vertextable.Tangent.fValue[2] = tangent.fValue[2];

					vertextable.vVertexIndex.push_back(uFaceCount * 3 + i);

					mVertexTable[uValue] = vertextable;
				}
			}

			//
			uFaceCount++;
		}
	}

	inf.close();

	//
	map<UINT, sVertexTable>::iterator it = mVertexTable.begin();
	for (;it != mVertexTable.end();++it)
	{
		for (UINT i = 0;i < it->second.vVertexIndex.size();++i)
		{
			vVertex[it->second.vVertexIndex[i]].vTangent = it->second.Tangent;

			sPosition& tangent = vVertex[it->second.vVertexIndex[i]].vTangent;
			float fLength = sqrt(tangent.fValue[0] * tangent.fValue[0] + tangent.fValue[1] * tangent.fValue[1] + tangent.fValue[2] * tangent.fValue[2]);
			tangent.fValue[0] /= fLength + EPSILON;
			tangent.fValue[1] /= fLength + EPSILON;
			tangent.fValue[2] /= fLength + EPSILON;
		}
	}
}

UINT8 GeometryData[20480000];
void XObjResource::LoadFromFile()
{
	XGeometry *pGeometry = XGeometryManager::GetResource(L"entity");
	if (!pGeometry)
	{
		vector<sVertex> vVertex;
		vector<UINT> vIndex;
		ReadDataFromObjFile(L"Media\\entity.obj", vVertex, vIndex);

		UINT uSize = vVertex.size()*sizeof(sVertex) + vIndex.size()*sizeof(UINT);

		UINT uOffset = 0;
		memcpy(GeometryData + uOffset, &(vVertex[0]), vVertex.size()*sizeof(sVertex));
		uOffset = vVertex.size()*sizeof(sVertex);
		memcpy(GeometryData + uOffset, &(vIndex[0]), vIndex.size()*sizeof(UINT));

		pEntity->InitGeometry(L"entity", vVertex.size(), sizeof(sVertex), vIndex.size(), DXGI_FORMAT_R32_UINT, GeometryData);

		//
		//delete[] pGeometryData;

		return;
	}

	//
	pEntity->m_pGeometry = pGeometry;
}