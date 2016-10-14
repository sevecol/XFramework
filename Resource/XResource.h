
#pragma once

#include <Windows.h>
#include <string>

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