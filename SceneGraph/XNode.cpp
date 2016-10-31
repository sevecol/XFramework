
#include "XNode.h"

XNode::XNode()
{
	m_bVisiable = true;
	m_fPosX = m_fPosY = m_fPosZ = 0.0f;
	m_fScale = 1.0f;
}

void XNode::SetVisiable(bool bVisiable)
{
	m_bVisiable = bVisiable;
}
bool XNode::GetVisable()
{
	return m_bVisiable;
}

void XNode::SetPos(float x, float y, float z)
{
	m_fPosX = x;
	m_fPosY = y;
	m_fPosZ = z;
}
void XNode::GetPos(float& x, float& y, float& z)
{
	x = m_fPosX;
	y = m_fPosY;
	z = m_fPosZ;
}
void XNode::SetScale(float s)
{
	m_fScale = s;
}
float XNode::GetScale()
{
	return m_fScale;
}