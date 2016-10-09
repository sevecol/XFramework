
#include "XBufferManager.h"
#include "XFrameResource.h"

#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

#include "..\Thread\XResourceThread.h"

#define BLOCKBUFFER_SIZE			1024*1024
#define BLOCKBUFFER_NUM				20

extern ComPtr<ID3D12Device>			g_pDevice;
extern ComPtr<ID3D12Fence>			g_pFence;
extern UINT							g_uFrameIndex;
extern XFrameResource				g_FrameResource[3];

extern XResourceThread				g_ResourceThread;

XBufferManager						g_BufferManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XBufferManager::XBufferManager():m_pBlockBuffer(nullptr)
{
}
XBufferManager::~XBufferManager()
{
	SAFE_DELGRP(m_pBlockBuffer);
	if (m_pBlockBufferDevice)
	{
		m_pBlockBufferDevice->Unmap(0, nullptr);
	}
}

void XBufferManager::Init(ID3D12Device* pDevice)
{
	UINT64 uCpuAddress = 0;

	// Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(BLOCKBUFFER_SIZE*BLOCKBUFFER_NUM),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pBlockBufferDevice)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	ThrowIfFailed(m_pBlockBufferDevice->Map(0, nullptr, reinterpret_cast<void**>(&uCpuAddress)));

	//
	m_pBlockBuffer = new XBlockBuffer[BLOCKBUFFER_NUM];
	for (UINT32 i = 0;i < BLOCKBUFFER_NUM;++i)
	{
		m_pBlockBuffer[i].m_uCpuAddress = uCpuAddress + i*BLOCKBUFFER_SIZE;
		m_pBlockBuffer[i].m_uGpuAddress = m_pBlockBufferDevice->GetGPUVirtualAddress() + i*BLOCKBUFFER_SIZE;

		m_lFreeBlock.push_back(&m_pBlockBuffer[i]);
	}
}
void XBufferManager::Update()
{
	std::list<XBlockBuffer*>::iterator it = m_lWaitBlock.begin();
	while (it != m_lWaitBlock.end())
	{
		UINT64 uComplateFenceValue = g_pFence->GetCompletedValue();
		if ((*it)->m_uFenceValue <= uComplateFenceValue)
		{
			m_lFreeBlock.push_back((*it));
			it = m_lWaitBlock.erase(it);
		}
		else
		{
			it++;
		}
	}
}

IBuffer* XBufferManager::CreateBuffer(eBufferType eType, UINT32 uSize, UINT8 *pData)
{
	switch (eType)
	{
	case EBUFFERTYPE_STATIC:
	{
		XStaticBuffer *pBuffer = new XStaticBuffer;

		// Create the buffer.
		ThrowIfFailed(g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pBuffer->m_pBuffer)));

		ThrowIfFailed(g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pBuffer->m_pBufferUploadHeap)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = pData;
		vertexData.RowPitch = uSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(g_ResourceThread.GetResourceCommandList(), pBuffer->m_pBuffer.Get(), pBuffer->m_pBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		g_ResourceThread.GetResourceCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pBuffer->m_pBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		return pBuffer;
	}
	break;
	case EBUFFERTYPE_BLOCK:
	{
		if (uSize <= BLOCKBUFFER_SIZE)
		{
			if (m_lFreeBlock.size())
			{
				XBlockBuffer *pBuffer = *(m_lFreeBlock.begin());
				m_lFreeBlock.pop_front();

				if (pBuffer)
				{
					memcpy((void*)(pBuffer->m_uCpuAddress), pData, uSize);
					return pBuffer;
				}
			}
		}
		return nullptr;
	}
	break;
	}

	return NULL;
}

bool XBufferManager::DeleteBuffer(IBuffer*& pIBuffer)
{
	if (pIBuffer == nullptr)
		return false;

	//
	switch (pIBuffer->GetType())
	{
	case EBUFFERTYPE_STATIC:
	{
		SAFE_DELETE(pIBuffer);
		pIBuffer = nullptr;
	}
	break;
	case EBUFFERTYPE_BLOCK:
	{
		XBlockBuffer *pBlockBuffer = dynamic_cast<XBlockBuffer*>(pIBuffer);
		if (pBlockBuffer)
		{
			pBlockBuffer->m_uFenceValue = g_FrameResource[g_uFrameIndex].m_uFenceValue;
			m_lWaitBlock.push_back(pBlockBuffer);
		}
		pIBuffer = nullptr;
	}
	break;
	}

	return true;
}