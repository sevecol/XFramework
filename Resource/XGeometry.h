
#pragma once

#include "XResource.h"
#include "XBuffer.h"

#include "..\Math\Vector3.h"
#include <map>

//
struct XGeometry : public XResource
{
	IBuffer							*m_pBuffer;
	D3D12_VERTEX_BUFFER_VIEW		m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW			m_IndexBufferView;

	//
	UINT							m_uNumIndices, m_uNumVertexs;
	Vector3f						m_vMax, m_vMin;
private:
	static std::map<std::wstring, XGeometry*>		m_mGeometry;

public:
	XGeometry(LPCWSTR pName) : XResource(pName),m_pBuffer(nullptr), m_uNumIndices(0){}
	~XGeometry();

	virtual D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &m_VertexBufferView; }
	virtual D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &m_IndexBufferView; }
	virtual UINT GetNumIndices() { return m_uNumIndices; }
	virtual UINT GetNumVertexs() { return m_uNumVertexs; }

	//
	static XGeometry* GetGeometry(LPCWSTR pName);
	static XGeometry* CreateGeometry(LPCWSTR pName,UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData);
	static void DeleteGeometry(XGeometry** ppGeometry);
};

