
#pragma once

#include "..\..\Instance\XEntity.h"
#include "..\..\Thread\XResourceThread.h"
#include "..\..\DXSampleHelper.h"

class XBinResource : public IResourceLoad
{
	UINT8* m_pMeshData;
	UINT m_meshDataLength;
	LPWSTR m_pFileName;
public:
	XEntity *m_pEntity;
	
	XBinResource(LPWSTR pFileName) :m_pFileName(pFileName) {};

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