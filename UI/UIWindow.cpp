
#include "UIWindow.h"
#include "..\DXSampleHelper.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT32 UIWindow::m_uInstanceCount = 1;
UINT32 UIWindow::m_uInstanceStart = 0;

UIWindow::UIWindow():m_pParent(nullptr),m_bVisable(true),m_uX(0),m_uY(0),m_uWidth(0),m_uHeight(0){}
UIWindow::~UIWindow()
{
	for (unsigned int i = 0;i < m_vChildren.size();++i)
	{
		delete m_vChildren[i];
	}
	m_vChildren.clear();
}

void UIWindow::Update()
{
	//
	if (!IsVisable())
	{
		return;
	}
	for (unsigned int i = 0;i < m_vChildren.size();++i)
	{
		m_vChildren[i]->Update();
	}
}
void UIWindow::Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	if (!IsVisable())
	{
		return;
	}
	for (unsigned int i = 0;i < m_vChildren.size();++i)
	{
		m_vChildren[i]->Render(pCommandList, uFenceValue);
	}
}
bool UIWindow::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!IsVisable())
	{
		return false;
	}
	for (unsigned int i = 0;i < m_vChildren.size();++i)
	{
		m_vChildren[i]->ProcessMessage(message, wParam, lParam);
	}
	return false;
}
void UIWindow::SetInfo(UIWindowInfo& info)
{
	info.m_uWidth	= m_uWidth / 1264.0f;
	info.m_uHeight	= m_uHeight / 622.0f;
	info.m_uX		= -1.0f * (1.0f - info.m_uWidth ) + m_uX / 632.0f;
	info.m_uY		=  1.0f * (1.0f - info.m_uHeight) - m_uY / 311.0f;
}

void UIWindow::Move(UINT16 uX, UINT16 uY)
{
	m_uX		= uX;
	m_uY		= uY;
}
void UIWindow::SetWidthHeight(UINT16 uWidth, UINT16 uHeight)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
}
void UIWindow::SetVisable(bool bVisable)
{
	m_bVisable = bVisable;
}
bool UIWindow::IsVisable()
{
	return m_bVisable;
}

void UIWindow::AddChild(UIWindow *pChild, UINT16 uX, UINT16 uY)
{
	std::vector<UIWindow*>::const_iterator it = find(m_vChildren.begin(), m_vChildren.end(), pChild);
	if (it == m_vChildren.end())
	{
		pChild->m_pParent = this;
		pChild->Move(uX, uY);
		m_vChildren.push_back(pChild);
	}
}
void UIWindow::DelChild(UIWindow *pChild)
{
	std::vector<UIWindow*>::iterator it = find(m_vChildren.begin(), m_vChildren.end(), pChild);
	if (it != m_vChildren.end())
	{
		pChild->m_pParent = nullptr;
		m_vChildren.erase(it);
	}
}