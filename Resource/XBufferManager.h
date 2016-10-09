
#pragma once

#include "XBuffer.h"

//
class XBufferManager
{
	//
	ComPtr<ID3D12Resource>			m_pBlockBufferDevice;
	XBlockBuffer					*m_pBlockBuffer;
	std::list<XBlockBuffer*>		m_lFreeBlock;
	std::list<XBlockBuffer*>		m_lWaitBlock;
public:
	XBufferManager();
	~XBufferManager();

	void Init(ID3D12Device* pDevice);
	void Update();

	IBuffer* CreateBuffer(eBufferType eType, UINT32 uSize, UINT8 *pData);
	bool DeleteBuffer(IBuffer*& pIBuffer);
};