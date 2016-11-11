
#pragma once

#include "..\XDirectX12.h"

enum eRenderPath
{
	// Main
	// Pre
	ERENDERPATH_SHADOWMAP = 0,
	// 
	ERENDERPATH_GEOMETRY,
	ERENDERPATH_ALPHABLEND,
	// Post
	ERENDERPATH_POST,

	// 
	ERENDERPATH_VOXEL,

	ERENDERPATH_COUNT
};

class XNode
{
	bool				m_bVisiable;
	UINT				m_uRenderPathFlag;

	float				m_fPosX, m_fPosY, m_fPosZ;
	float				m_fRotationX, m_fRotationY, m_fRotationZ;
	float				m_fScale;
public:
	XNode();
	virtual ~XNode() {};

	virtual void Render(ID3D12GraphicsCommandList* pCommandList, eRenderPath ePath, UINT64 uFenceValue) {};
	virtual void Update() {};

	void SetVisiable(bool bVisiable);
	bool GetVisable();
	void SetRenderPathFlag(UINT uRenderPathFlag);
	UINT GetRenderPathFlag();

	void SetPos(float x, float y, float z);
	void GetPos(float& x, float& y, float& z);
	void SetRotation(float x, float y, float z);
	void GetRotation(float& x, float& y, float& z);
	void SetScale(float s);
	float GetScale();
};