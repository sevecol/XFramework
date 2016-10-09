
#pragma once

#include <d2d1_3.h>
#include <dwrite.h>
#include <dxgi1_4.h>
#include <d3d11on12.h>

#include <wrl.h>
using namespace Microsoft::WRL;

struct DirectX11Layer
{
	static const UINT FrameCount = 3;

	//
	ComPtr<ID3D11On12Device>			m_p11On12Device;
	ComPtr<ID3D11DeviceContext>			m_pDeviceContext;
	ComPtr<ID2D1DeviceContext2>			m_d2dDeviceContext;
	ComPtr<IDWriteFactory>				m_dWriteFactory;
	ComPtr<ID2D1Factory3>				m_d2dFactory;
	ComPtr<ID2D1Device2>				m_d2dDevice;
	ComPtr<ID3D11Resource>				m_wrappedBackBuffers[FrameCount];
	ComPtr<ID2D1Bitmap1>				m_d2dRenderTargets[FrameCount];
	ComPtr<ID2D1SolidColorBrush>		m_textBrush;
	ComPtr<IDWriteTextFormat>			m_textFormat;
};