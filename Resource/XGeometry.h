
#pragma once

#include "XBuffer.h"
#include "..\Math\Vector3.h"

//
struct XGeometry
{
	IBuffer							*m_pBuffer;
	D3D12_VERTEX_BUFFER_VIEW		m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW			m_IndexBufferView;

	//
	UINT							m_uNumIndices, m_uNumVertexs;

public:
	Vector3f						m_vMax, m_vMin;

public:
	XGeometry() : m_pBuffer(nullptr), m_uNumIndices(0){}
	~XGeometry();

	virtual D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &m_VertexBufferView; }
	virtual D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &m_IndexBufferView; }
	virtual UINT GetNumIndices() { return m_uNumIndices; }
	virtual UINT GetNumVertexs() { return m_uNumVertexs; }
};

XGeometry* CreateGeometry(UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData);