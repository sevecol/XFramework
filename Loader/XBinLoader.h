
#include "occcity.h"
#include "..\XEntity.h"
#include "..\Thread\XResourceThread.h"
#include "..\DXSampleHelper.h"

class XBinResource : public IResourceLoad
{
public:
	UINT8* pMeshData;
	UINT meshDataLength;
	XEntity *pEntity;

	virtual void LoadFromFile()
	{
		ReadDataFromFile(L"occcity.bin", &pMeshData, &meshDataLength);
		pEntity->InitGeometry(L"111", SampleAssets::VertexDataSize / SampleAssets::StandardVertexStride, SampleAssets::StandardVertexStride, SampleAssets::IndexDataSize / 4, DXGI_FORMAT_R32_UINT, pMeshData + SampleAssets::VertexDataOffset);
	}
	virtual void PostLoad()
	{
		pEntity->IncreaseResourceComplate();
	}
	virtual bool IsNeedWaitForResource()
	{
		return true;
	}
};