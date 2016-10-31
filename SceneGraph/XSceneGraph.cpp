
#include "XSceneGraph.h"

void XSceneGraph::AddNode(eRenderPath eRenderPath,XNode* pNode)
{
	m_vNodes[eRenderPath].push_back(pNode);
}
void XSceneGraph::Update()
{
	for (unsigned int i = 0;i < ERENDERPATH_COUNT;++i)
	{
		for (unsigned int j = 0;j < m_vNodes[i].size();++j)
		{
			m_vNodes[i][j]->Update();
		}
	}
}
void XSceneGraph::Render(eRenderPath eRenderPath,ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	for (unsigned int i = 0;i < m_vNodes[eRenderPath].size();++i)
	{
		m_vNodes[eRenderPath][i]->Render(pCommandList, uFenceValue);
	}
}

void XSceneGraph::Clean()
{
	for (unsigned int i = 0;i < ERENDERPATH_COUNT;++i)
	{
		for (unsigned int j = 0;j < m_vNodes[i].size();++j)
		{
			SAFE_DELETE(m_vNodes[i][j]);
		}
		m_vNodes[i].clear();
	}
}