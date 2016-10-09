
#include "XGeometry.h"
#include "XBufferManager.h"

#include "..\DXSampleHelper.h"

extern XBufferManager		g_BufferManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XGeometry::~XGeometry()
{
	g_BufferManager.DeleteBuffer(m_pBuffer);
}

XGeometry* CreateGeometry(UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData)
{
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

	XGeometry *pGeometry = new XGeometry;

	//
	UINT uBufferSize = uVertexCount * uVertexStride + uIndexCount * uIndexSize;
	pGeometry->m_pBuffer = g_BufferManager.CreateBuffer(EBUFFERTYPE_BLOCK, uBufferSize, pGeometryData);
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