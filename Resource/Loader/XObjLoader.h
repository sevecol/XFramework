
#pragma once

#include "..\..\Instance\XEntity.h"
#include "..\..\Thread\XResourceThread.h"
#include "..\..\DXSampleHelper.h"

class XObjResource : public IResourceLoad
{
	LPWSTR m_pFileName;
public:
	XEntity *m_pEntity;

	XObjResource(LPWSTR pFileName) :m_pFileName(pFileName) {};

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