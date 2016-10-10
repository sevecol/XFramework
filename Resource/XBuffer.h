
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

template<typename T>
class XStructuredBuffer
{
	ComPtr<ID3D12Resource>			m_pBuffer;
	UINT							m_uSIndex;
public:
	XStructuredBuffer(ID3D12Device* pDevicie, UINT uCounts, D3D12_CPU_DESCRIPTOR_HANDLE& Descriptor)
	{
		// Create the buffer.
		ThrowIfFailed(g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uCounts*sizeof(T), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer)));

		//
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = uCounts;
		UDesc.Buffer.StructureByteStride = sizeof(T);
		UDesc.Buffer.CounterOffsetInBytes = 0;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		pDevicie->CreateUnorderedAccessView(m_pBuffer.Get(), nullptr, &UDesc, Descriptor);
	}
	~XStructuredBuffer()
	{
	}

	//
	ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }
	UINT GetSIndex() { return m_uSIndex; }
};