
#include "XBuffer.h"

D3D12_GPU_VIRTUAL_ADDRESS XStaticBuffer::GetGpuAddress()
{
	if (m_pBuffer)
	{
		return m_pBuffer->GetGPUVirtualAddress();
	}
	return 0;
}