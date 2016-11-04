
#pragma once

#include "..\..\Instance\XEntity.h"
#include "..\..\Thread\XResourceThread.h"
#include "..\..\DXSampleHelper.h"

class XVertexIndexResource : public IResourceLoad
{
	LPWSTR m_pFileName;
public:
	XEntity *m_pEntity;

	XVertexIndexResource(LPWSTR pFileName) :m_pFileName(pFileName) {};

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