
#pragma once

#include "UIWindow.h"

//#include "..\Resource\Texture.h"

class UIImgWindow : public UIWindow
{
public:
	//ITexture						*m_pTexture;
public:
	UIImgWindow();
	virtual ~UIImgWindow();

	//
	virtual UINT GetSrvIndex();

	virtual void Update();
	virtual void Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue);
};