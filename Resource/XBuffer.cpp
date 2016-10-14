
#include "XBuffer.h"
#include "XFrameResource.h"

#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"

#include "..\Thread\XResourceThread.h"

#define BLOCKBUFFER_SIZE			1024*1024
#define BLOCKBUFFER_NUM				20

extern XEngine						*g_pEngine;
extern XResourceThread				*g_pResourceThread;

extern UINT							g_uFrameIndex;
extern XFrameResource				*g_pFrameResource[FRAME_NUM];

D3D12_GPU_VIRTUAL_ADDRESS XStaticBuffer::GetGpuAddress()
{
	if (m_pBuffer)
	{
		return m_pBuffer->GetGPUVirtualAddress();
	}
	return 0;
}

XBlockBuffer* XBuffer::m_pBlockBuffer = nullptr;
ID3D12Resource* XBuffer::m_pBlockBufferDevice = nullptr;
std::list<XBlockBuffer*> XBuffer::m_lFreeBlock;
std::list<XBlockBuffer*> XBuffer::m_lWaitBlock;
void XBuffer::Clean()
{
	SAFE_DELGRP(m_pBlockBuffer);
	if (m_pBlockBufferDevice)
	{
		m_pBlockBufferDevice->Unmap(0, nullptr);
		SAFE_RELEASE(m_pBlockBufferDevice);
	}
}

void XBuffer::Init(ID3D12Device* pDevice)
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
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_pBlockBufferDevice->Map(0, &readRange, reinterpret_cast<void**>(&uCpuAddress)));

	//
	m_pBlockBuffer = new XBlockBuffer[BLOCKBUFFER_NUM];
	for (UINT32 i = 0;i < BLOCKBUFFER_NUM;++i)
	{
		m_pBlockBuffer[i].m_uCpuAddress = uCpuAddress + i*BLOCKBUFFER_SIZE;
		m_pBlockBuffer[i].m_uGpuAddress = m_pBlockBufferDevice->GetGPUVirtualAddress() + i*BLOCKBUFFER_SIZE;

		m_lFreeBlock.push_back(&m_pBlockBuffer[i]);
	}
}
void XBuffer::Update()
{
	std::list<XBlockBuffer*>::iterator it = m_lWaitBlock.begin();
	while (it != m_lWaitBlock.end())
	{
		UINT64 uComplateFenceValue = g_pEngine->m_pFence->GetCompletedValue();
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

IBuffer* XBuffer::CreateBuffer(eBufferType eType, UINT32 uSize, UINT8 *pData)
{
	switch (eType)
	{
	case EBUFFERTYPE_STATIC:
	{
		XStaticBuffer *pBuffer = new XStaticBuffer;

		// Create the buffer.
		ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pBuffer->m_pBuffer)));

		ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
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

		UpdateSubresources<1>(g_pResourceThread->GetResourceCommandList(), pBuffer->m_pBuffer.Get(), pBuffer->m_pBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		g_pResourceThread->GetResourceCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pBuffer->m_pBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

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

bool XBuffer::DeleteBuffer(IBuffer*& pIBuffer)
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
			pBlockBuffer->m_uFenceValue = g_pFrameResource[g_uFrameIndex]->m_uFenceValue;
			m_lWaitBlock.push_back(pBlockBuffer);
		}
		pIBuffer = nullptr;
	}
	break;
	}

	return true;
}