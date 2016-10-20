
#include "XGeometry.h"
#include "XBuffer.h"

#include "..\DXSampleHelper.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::map<std::wstring, XGeometry*> XGeometry::m_mGeometry;
XGeometry::~XGeometry()
{
	XBuffer::DeleteBuffer(m_pBuffer);
}

XGeometry* XGeometry::CreateGeometry(LPCWSTR pName,UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData)
{
	//
	XGeometry *pGeometry = nullptr;
	std::map<std::wstring, XGeometry*>::iterator it = XGeometry::m_mGeometry.find(pName);
	if (it != XGeometry::m_mGeometry.end())
	{
		pGeometry = it->second;
		if (pGeometry)
		{
			pGeometry->AddRef();
		}
		return pGeometry;
	}

	if (!uVertexCount)
	{
		return nullptr;
	}

	//
	UINT uIndexSize = 0;
	switch (uIndexFormat)
	{
	case DXGI_FORMAT_R32_UINT:
		uIndexSize = 4;
		break;
	}
	if (!uIndexSize)
	{
		return nullptr;
	}

	pGeometry = new XGeometry(pName);
	XGeometry::m_mGeometry[pName] = pGeometry;

	//
	UINT uBufferSize = uVertexCount * uVertexStride + uIndexCount * uIndexSize;
	pGeometry->m_pBuffer = XBuffer::CreateBuffer(EBUFFERTYPE_BLOCK, uBufferSize, pGeometryData);
	if (!pGeometry->m_pBuffer)
	{
		delete pGeometry;
		return nullptr;
	}

	// Initialize the vertex buffer view.
	pGeometry->m_uNumVertexs = uVertexCount;
	pGeometry->m_VertexBufferView.BufferLocation = pGeometry->m_pBuffer->GetGpuAddress();
	pGeometry->m_VertexBufferView.StrideInBytes = uVertexStride;
	pGeometry->m_VertexBufferView.SizeInBytes = uVertexCount * uVertexStride;

	//
	pGeometry->m_uNumIndices = uIndexCount;
	if (uIndexCount)
	{
		// Describe the index buffer view.
		pGeometry->m_IndexBufferView.BufferLocation = pGeometry->m_VertexBufferView.BufferLocation + uVertexCount * uVertexStride;
		pGeometry->m_IndexBufferView.Format = (DXGI_FORMAT)uIndexFormat;
		pGeometry->m_IndexBufferView.SizeInBytes = uIndexCount * uIndexSize;
	}

	//
/*
	struct VSInput
	{
		float position[3];
		float normal[3];
		float uv[2];
		float tangent[3];
	};
	VSInput *pInput = (VSInput*)pGeometryData;
*/
	//
	pGeometry->m_vMin = Vector3f(10000, 10000, 10000);
	pGeometry->m_vMax = Vector3f(-10000, -10000, -10000);
	for (unsigned int i = 0;i < uVertexCount;++i)
	{
		Vector3f *pPos = (Vector3f*)(pGeometryData + i*uVertexStride);

		if (pPos->x < pGeometry->m_vMin.x)
		{
			pGeometry->m_vMin.x = pPos->x;
		}
		if (pPos->y < pGeometry->m_vMin.y)
		{
			pGeometry->m_vMin.y = pPos->y;
		}
		if (pPos->z < pGeometry->m_vMin.z)
		{
			pGeometry->m_vMin.z = pPos->z;
		}

		if (pPos->x > pGeometry->m_vMax.x)
		{
			pGeometry->m_vMax.x = pPos->x;
		}
		if (pPos->y > pGeometry->m_vMax.y)
		{
			pGeometry->m_vMax.y = pPos->y;
		}
		if (pPos->z > pGeometry->m_vMax.z)
		{
			pGeometry->m_vMax.z = pPos->z;
		}
	}

	return pGeometry;
}

void XGeometry::DeleteGeometry(XGeometry** ppGeometry)
{
	if (*ppGeometry)
	{
		int iRef = (*ppGeometry)->DecRef();
		if (iRef <= 0)
		{
			std::map<std::wstring, XGeometry*>::iterator it = XGeometry::m_mGeometry.find((*ppGeometry)->GetName());
			if (it != XGeometry::m_mGeometry.end())
			{
				XGeometry::m_mGeometry.erase(it);
			}
			SAFE_DELETE(*ppGeometry);
		}
		*ppGeometry = nullptr;
	}
}