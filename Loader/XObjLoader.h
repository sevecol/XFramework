
#pragma once

#include "..\XEntity.h"
#include "..\Thread\XResourceThread.h"
#include "..\DXSampleHelper.h"

class XObjResource : public IResourceLoad
{
public:
	UINT8* pMeshData;
	UINT meshDataLength;
	XEntity *pEntity;

	virtual void LoadFromFile();
	virtual void PostLoad()
	{
		//pEntity->IncreaseResourceComplate();
	}
	virtual bool IsNeedWaitForResource()
	{
		return true;
	}
};