
#include "XSceneGraph.h"
#include "..\Resource\XResource.h"

void XSceneGraph::AddNode(XNode* pNode)
{
	m_vNodes.push_back(pNode);
}
void XSceneGraph::Update()
{
	for (unsigned int i = 0;i < m_vNodes.size();++i)
	{
		m_vNodes[i]->Update();
	}
}
void XSceneGraph::Render(eRenderPath ePath,ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	for (unsigned int i = 0;i < m_vNodes.size();++i)
	{
		XNode *pNode = m_vNodes[i];
		if ((pNode->GetVisable())&&(pNode->GetRenderPathFlag()&(1<<ePath)))
		{
			pNode->Render(pCommandList, ePath, uFenceValue);
		}
	}
}

void XSceneGraph::Clean()
{
	for (unsigned int i = 0;i < m_vNodes.size();++i)
	{
		SAFE_DELETE(m_vNodes[i]);
	}
	m_vNodes.clear();
}