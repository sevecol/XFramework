
#pragma once

#include "..\XDirectX12.h"

class XNode
{
	bool				m_bVisiable;

	float				m_fPosX, m_fPosY, m_fPosZ;
	float				m_fScale;
public:
	XNode();
	virtual ~XNode() {};

	virtual void Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue) {};
	virtual void Update() {};

	void SetVisiable(bool bVisiable);
	bool GetVisable();

	void SetPos(float x, float y, float z);
	void GetPos(float& x, float& y, float& z);
	void SetScale(float s);
	float GetScale();
};