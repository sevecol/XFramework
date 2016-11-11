
#pragma once

#include "XNode.h"
#include <vector>

class XSceneGraph
{
	std::vector<XNode*>		m_vNodes;
public:
	void AddNode(XNode* pNode);
	
	void Render(eRenderPath ePath,ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue);
	void Update();
	void Clean();
};