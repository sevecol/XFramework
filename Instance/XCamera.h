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

#pragma once

#include "..\Math\OptFrustum.h"

#include <windows.h>
#include <DirectXMath.h>
using namespace DirectX;

class XCamera 
{
public:
	XCamera();

	//
	void Init(float fov, float aspectRatio, float nearPlane = 1.0f, float farPlane = 1000.0f);
	bool ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	//
	void InitPos(XMFLOAT3 position);
	void Update(float elapsedSeconds);
	XMMATRIX GetViewMatrix();
	XMMATRIX GetProjectionMatrix();
	void SetMoveSpeed(float unitsPerSecond);
	void SetTurnSpeed(float radiansPerSecond);

	OptFrustum* GetFrustum();
	XMFLOAT3* GetPosition();
	float GetNear();
	float GetFar();

private:
	void Reset();

	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
	void OnMouseLDown(LPARAM param);
	void OnMouseLUp(LPARAM param);
	void OnMouseMove(LPARAM param);

	struct KeysPressed
	{
		bool w;
		bool a;
		bool s;
		bool d;

		bool left;
		bool right;
		bool up;
		bool down;

		float mleft;
		float mright;
		float mup;
		float mdown;
	};

	XMFLOAT3 m_initialPosition;
	XMFLOAT3 m_position;
	float m_yaw;				// Relative to the +z axis.
	float m_pitch;				// Relative to the xz plane.
	XMFLOAT3 m_lookDirection;
	XMFLOAT3 m_upDirection;
	float m_moveSpeed;			// Speed at which the camera moves, in units per second.
	float m_turnSpeed;			// Speed at which the camera turns, in radians per second.

	float m_fov, m_aspectRatio, m_nearPlane, m_farPlane;

	KeysPressed m_keysPressed;

	//
	OptFrustum m_frustum;
};
