//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "XCamera.h"

XCamera::XCamera():
	m_initialPosition(0, 0, 0),
	m_position(m_initialPosition),
	m_yaw(XM_PI),
	m_pitch(0.0f),
	m_lookDirection(0, 0, -1),
	m_upDirection(0, 1, 0),
	m_moveSpeed(20.0f),
	m_turnSpeed(XM_PIDIV2),
	m_fov(0.0f), m_aspectRatio(0.0f), m_nearPlane(1.0f), m_farPlane(1000.0f)
{
	ZeroMemory(&m_keysPressed, sizeof(m_keysPressed));
}

void XCamera::Init(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	m_fov = fov;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
}

bool XCamera::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Camera
	switch (message)
	{
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		break;
	case WM_KEYUP:
		OnKeyUp(wParam);
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(lParam);
		break;
	case WM_LBUTTONDOWN:
		OnMouseLDown(lParam);
		break;
	case WM_LBUTTONUP:
		OnMouseLUp(lParam);
		break;
	}

	return true;
}

void XCamera::InitPos(XMFLOAT3 position)
{
	m_initialPosition = position;
	Reset();
}

void XCamera::SetMoveSpeed(float unitsPerSecond)
{
	m_moveSpeed = unitsPerSecond;
}

void XCamera::SetTurnSpeed(float radiansPerSecond)
{
	m_turnSpeed = radiansPerSecond;
}

void XCamera::Reset()
{
	m_position = m_initialPosition;
	m_yaw = XM_PI;
	m_pitch = 0.0f;
	m_lookDirection = { 0, 0, -1 };
}

void XCamera::Update(float elapsedSeconds)
{
	// Calculate the move vector in camera space.
	XMFLOAT3 move(0, 0, 0);

	if (m_keysPressed.a)
		move.x -= 1.0f;
	if (m_keysPressed.d)
		move.x += 1.0f;
	if (m_keysPressed.w)
		move.z -= 1.0f;
	if (m_keysPressed.s)
		move.z += 1.0f;

	if (fabs(move.x) > 0.1f && fabs(move.z) > 0.1f)
	{
		XMVECTOR vector = XMVector3Normalize(XMLoadFloat3(&move));
		move.x = XMVectorGetX(vector);
		move.z = XMVectorGetZ(vector);
	}

	float moveInterval = m_moveSpeed * elapsedSeconds;
	float rotateInterval = m_turnSpeed * elapsedSeconds;

	if (m_keysPressed.left)
		m_yaw += rotateInterval;
	if (m_keysPressed.right)
		m_yaw -= rotateInterval;
	if (m_keysPressed.up)
		m_pitch += rotateInterval;
	if (m_keysPressed.down)
		m_pitch -= rotateInterval;

	if (m_keysPressed.mleft)
		m_yaw += m_keysPressed.mleft * rotateInterval;
	if (m_keysPressed.mright)
		m_yaw -= m_keysPressed.mright * rotateInterval;
	if (m_keysPressed.mup)
		m_pitch += m_keysPressed.mup * rotateInterval;
	if (m_keysPressed.mdown)
		m_pitch -= m_keysPressed.mdown * rotateInterval;

	//m_yaw += 0.11001f;
	m_keysPressed.mleft = m_keysPressed.mright = m_keysPressed.mup = m_keysPressed.mdown = 0.0f;

	// Prevent looking too far up or down.
	m_pitch = min(m_pitch, XM_PIDIV4);
	m_pitch = max(-XM_PIDIV4, m_pitch);

	// Move the camera in model space.
	float x = move.x * -cosf(m_yaw) - move.z * sinf(m_yaw);
	float z = move.x * sinf(m_yaw) - move.z * cosf(m_yaw);
	m_position.x += x * moveInterval;
	m_position.z += z * moveInterval;

	// Determine the look direction.
	float r = cosf(m_pitch);
	m_lookDirection.x = r * sinf(m_yaw);
	m_lookDirection.y = sinf(m_pitch);
	m_lookDirection.z = r * cosf(m_yaw);

	//
	XMMATRIX matView = XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
	XMMATRIX matProj = XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
	Matrix4 matViewPorj = (*((Matrix4*)&matView)) * (*((Matrix4*)&matProj));
	m_frustum.InitData(matViewPorj, true);
}

XMMATRIX XCamera::GetViewMatrix()
{
	return XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
}

XMMATRIX XCamera::GetProjectionMatrix()
{
	return XMMatrixPerspectiveFovRH(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
}

void XCamera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = true;
		break;
	case 'A':
		m_keysPressed.a = true;
		break;
	case 'S':
		m_keysPressed.s = true;
		break;
	case 'D':
		m_keysPressed.d = true;
		break;
	case VK_LEFT:
		m_keysPressed.left = true;
		break;
	case VK_RIGHT:
		m_keysPressed.right = true;
		break;
	case VK_UP:
		m_keysPressed.up = true;
		break;
	case VK_DOWN:
		m_keysPressed.down = true;
		break;
	case VK_ESCAPE:
		Reset();
		break;
	}
}

void XCamera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = false;
		break;
	case 'A':
		m_keysPressed.a = false;
		break;
	case 'S':
		m_keysPressed.s = false;
		break;
	case 'D':
		m_keysPressed.d = false;
		break;
	case VK_LEFT:
		m_keysPressed.left = false;
		break;
	case VK_RIGHT:
		m_keysPressed.right = false;
		break;
	case VK_UP:
		m_keysPressed.up = false;
		break;
	case VK_DOWN:
		m_keysPressed.down = false;
		break;
	}
}

bool bRotation = false;
int iMouseX, iMouseY;
void XCamera::OnMouseLDown(LPARAM param)
{
	bRotation = true;
	iMouseX = LOWORD(param);
	iMouseY = HIWORD(param);
}
void XCamera::OnMouseLUp(LPARAM param)
{
	bRotation = false;
}
void XCamera::OnMouseMove(LPARAM param)
{
	int iXPos = LOWORD(param);
	int iYPos = HIWORD(param);

	//
	float fScale = 10.0f;

	//
	if (bRotation)
	{
		int iDeltaX = iXPos - iMouseX;
		if (iDeltaX > 0)
		{
			m_keysPressed.mright = iDeltaX / fScale;
		}
		else
		{
			m_keysPressed.mleft = -iDeltaX / fScale;
		}
		int iDeltaY = iYPos - iMouseY;
		if (iDeltaY > 0)
		{
			m_keysPressed.mdown = iDeltaY / fScale;
		}
		else
		{
			m_keysPressed.mup = -iDeltaY / fScale;
		}

		//
		iMouseX = iXPos;
		iMouseY = iYPos;
	}
}

OptFrustum* XCamera::GetFrustum()
{
	return &m_frustum;
}
XMFLOAT3* XCamera::GetPosition()
{
	return &m_position;
}
float XCamera::GetNear()
{
	return m_nearPlane;
}
float XCamera::GetFar()
{
	return m_farPlane;
}
