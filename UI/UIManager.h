
#pragma once

#include "..\Resource\XShader.h"
#include "..\Resource\XGeometry.h"
#include "UIWindow.h"

#include <DirectXMath.h>
using namespace DirectX;
using namespace Microsoft::WRL;

#define UIWINDOWNUM_MAX			1024
#define UIWINDOWBUFF_NUM		(UIWINDOWNUM_MAX/UIWINDOWGROUPNUM_MAX)

class UIManager
{
	UIWindow					*m_pRootUIWindow;
	//std::list<UIWindow*>		m_lpPopUIWindow;

	XShader						*m_pShader;
	XGeometry					*m_pGeometry;
	
	//UINT16					m_uIndex;
	ComPtr<ID3D12Resource>		m_pConstantBufferUploadHeap;
	struct ConstantBuffer
	{
		UIWindowInfo			m_Info[UIWINDOWGROUPNUM_MAX];
	};
	ConstantBuffer*				m_pConstantBuffers[UIWINDOWBUFF_NUM];

public:

	UIManager();
	virtual ~UIManager();

	//virtual IUIWindow* GetRootUIWindow();
	//virtual IUIWindow* CreatePopUIWindow(UINT16 uX, UINT16 uY, UINT16 uWidth, UINT16 uHeight);
	virtual UIWindow* CreateUIImgWindow(UIWindow *pParent, LPCWSTR pImgFileName, UINT16 uX, UINT16 uY, UINT16 uWidth, UINT16 uHeight);
	virtual void DelUIWindow(UIWindow*& pUIWindow);
	virtual void DelUIWindows();

	void Init(ID3D12Device* pDevice, UINT uWidth, UINT uHeight);
	void Update();
	void Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue);
	bool ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	//
	XShader* GetShader() { return m_pShader; }
	XGeometry* GetGeometry() { return m_pGeometry; }
	void SetGeometry(XGeometry *pGeometry);
};