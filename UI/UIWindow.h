
#pragma once

#include "..\XDirectX12.h"
#include <DirectXMath.h>
using namespace DirectX;
using namespace Microsoft::WRL;

#define UIWINDOWGROUPNUM_MAX	16

struct UIWindowInfo
{
	FLOAT						m_uX, m_uY, m_uWidth, m_uHeight;
};
class UIWindow
{
	UINT16						m_uX, m_uY, m_uWidth, m_uHeight;
	UIWindow					*m_pParent;
	std::vector<UIWindow*>		m_vChildren;
	bool						m_bVisable;

public:

	//
	static UINT32				m_uInstanceCount, m_uInstanceStart;

	UIWindow();
	virtual ~UIWindow();

	virtual void Update();
	virtual void Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue);
	bool ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	virtual void Move(UINT16 uX, UINT16 uY);
	virtual void SetWidthHeight(UINT16 uWidth, UINT16 uHeight);
	virtual UINT GetSrvIndex() { return 0xFFFFFFFF; }
	void SetInfo(UIWindowInfo& info);

	virtual void SetVisable(bool bVisable);
	virtual bool IsVisable();

	void AddChild(UIWindow *pChild,UINT16 uX,UINT16 uY);
	void DelChild(UIWindow *pChild);
};