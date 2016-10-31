
#pragma once

#include <Windows.h>
#include <string>
#include <map>

class XResource
{
	std::wstring	m_sName;
	int				m_iRef;
public:
	XResource(LPCWSTR pName) :m_sName(pName), m_iRef(1) {}
	virtual ~XResource() {}

	const std::wstring& GetName() { return m_sName; }
	void AddRef() { m_iRef++; }
	int DecRef()
	{
		m_iRef--;
		return m_iRef;
	}
};

template<typename T>
class XResourceManager
{
	static std::map<std::wstring, T*>	m_mResource;
public:
	static T* GetResource(LPCWSTR pName)
	{
		T *pResource = nullptr;
		std::map<std::wstring, T*>::iterator it = m_mResource.find(pName);
		if (it != m_mResource.end())
		{
			pResource = it->second;
			if (pResource)
			{
				pResource->AddRef();
			}
			return pResource;
		}
		return nullptr;
	}
	static void AddResource(LPCWSTR pName, T* pResource)
	{
		m_mResource[pResource->GetName()] = pResource;
	}
	static void DelResource(T **ppResource)
	{
		if (*ppResource)
		{
			int iRef = (*ppResource)->DecRef();
			if (iRef <= 0)
			{
				std::map<std::wstring, T*>::iterator it = m_mResource.find((*ppResource)->GetName());
				if (it != m_mResource.end())
				{
					m_mResource.erase(it);
				}
				SAFE_DELETE(*ppResource);
			}
			*ppResource = nullptr;
		}
	}
};