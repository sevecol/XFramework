
#pragma once

#include "..\Math\Vector3.h"
#include "..\XDirectX12.h"

enum eBufferType
{
	EBUFFERTYPE_STATIC = 0,
	EBUFFERTYPE_BLOCK,
	EBUFFERTYPE_TEMP
};

class IBuffer
{
public:
	virtual ~IBuffer() {}
	virtual eBufferType GetType() = 0;
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() = 0;
};

class XBuffer : public IBuffer
{
protected:
	eBufferType		m_eType;

public:
	XBuffer(eBufferType eType):m_eType(eType){}
	eBufferType GetType() { return m_eType; }
};

//
struct XStaticBuffer : public XBuffer
{
	ComPtr<ID3D12Resource>			m_pBuffer, m_pBufferUploadHeap;

	XStaticBuffer() :XBuffer(EBUFFERTYPE_STATIC) {}
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress();
};

struct XBlockBuffer : public XBuffer
{
	UINT64							m_uCpuAddress,m_uGpuAddress;
	UINT64							m_uFenceValue;

	XBlockBuffer() :XBuffer(EBUFFERTYPE_BLOCK), m_uCpuAddress(0),m_uGpuAddress(0){}
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() { return m_uGpuAddress; }
};

struct XTempBuffer : public XBuffer
{
	UINT64							m_uAddress;

	XTempBuffer() :XBuffer(EBUFFERTYPE_TEMP), m_uAddress(0) {}
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() { return m_uAddress; }
};