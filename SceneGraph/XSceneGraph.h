
#pragma once

#include "XNode.h"
#include <vector>

enum eRenderPath
{
	ERENDERPATH_NORMAL	= 0,
	ERENDERPATH_ALPHABLEND,

	ERENDERPATH_COUNT
};

class XSceneGraph
{
	std::vector<XNode*>		m_vNodes[ERENDERPATH_COUNT];
public:
	void AddNode(eRenderPath eRenderPath,XNode* pNode);
	
	void Render(eRenderPath eRenderPath,ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue);
	void Update();
	void Clean();
};