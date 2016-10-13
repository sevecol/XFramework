
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

struct XBlockBuffer;
class XBuffer : public IBuffer
{
	static 	ID3D12Resource					*m_pBlockBufferDevice;
	static	XBlockBuffer					*m_pBlockBuffer;
	static	std::list<XBlockBuffer*>		m_lFreeBlock;
	static	std::list<XBlockBuffer*>		m_lWaitBlock;

protected:
	eBufferType								m_eType;

public:
	XBuffer(eBufferType eType):m_eType(eType){}
	eBufferType GetType() { return m_eType; }

	static void Init(ID3D12Device* pDevice);
	static void Clean();
	static void Update();

	static IBuffer* CreateBuffer(eBufferType eType, UINT32 uSize, UINT8 *pData);
	static bool DeleteBuffer(IBuffer*& pIBuffer);
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

class IStructuredBuffer
{
public:
	virtual ~IStructuredBuffer() {}

	virtual ID3D12Resource* GetResource() = 0;
	virtual void SetUAVGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE& Descriptor) = 0;
	virtual D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGpuHandle() = 0;
};

template<typename T>
class XStructuredBuffer : public IStructuredBuffer
{
	ComPtr<ID3D12Resource>			m_pBuffer;
	UINT							m_uSIndex;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_hUAVGpuHandle;
public:
	XStructuredBuffer(ID3D12Device* pDevicie, UINT uCounts)
	{
		// Create the buffer.
		ThrowIfFailed(g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uCounts*sizeof(T), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer)));
	}
	XStructuredBuffer(ID3D12Device* pDevice, UINT uCounts, D3D12_CPU_DESCRIPTOR_HANDLE& Descriptor, bool bCounter = false, ID3D12Resource *pCounterBuffer = nullptr)
	{
		//
		UINT uSize = uCounts*sizeof(T);
		UINT uCounterOffset = 0;
		
		// Create the buffer.
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer)));

		//
		D3D12_UNORDERED_ACCESS_VIEW_DESC UDesc = {};
		UDesc.Format = DXGI_FORMAT_UNKNOWN;
		UDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UDesc.Buffer.FirstElement = 0;
		UDesc.Buffer.NumElements = uCounts;
		UDesc.Buffer.StructureByteStride = sizeof(T);
		UDesc.Buffer.CounterOffsetInBytes = uCounterOffset;
		UDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		pDevice->CreateUnorderedAccessView(m_pBuffer.Get(), pCounterBuffer, &UDesc, Descriptor);
	}
	~XStructuredBuffer()
	{
	}

	//
	ID3D12Resource* GetResource() { return m_pBuffer.Get(); }
	UINT GetSIndex() { return m_uSIndex; }
	void SetUAVGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE& Descriptor)
	{
		m_hUAVGpuHandle = Descriptor;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGpuHandle() { return m_hUAVGpuHandle; }
};