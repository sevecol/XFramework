
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

void ReadDataFromObjFile(LPCWSTR filename, vector<sVertex>& vVertex, vector<UINT>& vIndex)
{
	ifstream inf;
	inf.open(filename);

	vector<sPosition> vPosition;
	vector<sPosition> vNormal;
	vector<sTexcoord> vTexcoord;

	//
	string sLine;
	string s1, s2, s3, s4;

	UINT uVertexCount = 0;
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
			UINT uIndex[3];
			sscanf(s2.c_str(), "%d/%d/%d", &uIndex[0], &uIndex[1], &uIndex[2]);

			sVertex vertex;
			vertex.vPosition = vPosition[uIndex[0] - 1];
			vertex.vTexcoord = vTexcoord[uIndex[1] - 1];
			vertex.vNormal = vNormal[uIndex[2] - 1];
			vVertex.push_back(vertex);
			vIndex.push_back(uVertexCount++);

			sscanf(s3.c_str(), "%d/%d/%d", &uIndex[0], &uIndex[1], &uIndex[2]);

			vertex.vPosition = vPosition[uIndex[0] - 1];
			vertex.vTexcoord = vTexcoord[uIndex[1] - 1];
			vertex.vNormal = vNormal[uIndex[2] - 1];
			vVertex.push_back(vertex);
			vIndex.push_back(uVertexCount++);

			sscanf(s4.c_str(), "%d/%d/%d", &uIndex[0], &uIndex[1], &uIndex[2]);

			vertex.vPosition = vPosition[uIndex[0] - 1];
			vertex.vTexcoord = vTexcoord[uIndex[1] - 1];
			vertex.vNormal = vNormal[uIndex[2] - 1];
			vVertex.push_back(vertex);
			vIndex.push_back(uVertexCount++);
		}
	}
}

UINT8 GeometryData[20480000];
void XObjResource::LoadFromFile()
{
	vector<sVertex> vVertex;
	vector<UINT> vIndex;
	ReadDataFromObjFile(L"entity.obj", vVertex, vIndex);

	UINT uSize = vVertex.size()*sizeof(sVertex) + vIndex.size()*sizeof(UINT);

	UINT uOffset = 0;
	memcpy(GeometryData + uOffset, &(vVertex[0]), vVertex.size()*sizeof(sVertex));
	uOffset = vVertex.size()*sizeof(sVertex);
	memcpy(GeometryData + uOffset, &(vIndex[0]), vIndex.size()*sizeof(UINT));

	pEntity->InitGeometry(L"entity", vVertex.size(), sizeof(sVertex), vIndex.size(), DXGI_FORMAT_R32_UINT, GeometryData);

	//
	//delete[] pGeometryData;
}