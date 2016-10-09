
#include "UIImgWindow.h"
#include "UIManager.h"

extern UIManager						g_UIManager;
extern ComPtr<ID3D12DescriptorHeap>		g_pCbvSrvUavHeap;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UIImgWindow::UIImgWindow(){}
UIImgWindow::~UIImgWindow()
{
}

UINT UIImgWindow::GetSrvIndex()
{
	return 0xFFFFFFFF;
}

void UIImgWindow::Update()
{
	//
	if (!IsVisable())
	{
		return;
	}
	//if (IsResourceComplate())
	//{
	//	g_UIManager.Add(this);
	//}
	
	//
	UIWindow::Update();
}
void UIImgWindow::Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	if (pCommandList)
	{
		//TextureManager *pTextureManager = GetXEngine()->GetTextureManager();
		ID3D12DescriptorHeap *ppHeaps[] = { g_pCbvSrvUavHeap.Get() };
		pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		pCommandList->SetGraphicsRootDescriptorTable(2, g_pCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

		//
		XGeometry* pGeometry = g_UIManager.GetGeometry();
		pCommandList->IASetVertexBuffers(0, 1, pGeometry->GetVertexBufferView());
		if (pGeometry->GetNumIndices())
		{
			pCommandList->IASetIndexBuffer(pGeometry->GetIndexBufferView());
			pCommandList->DrawIndexedInstanced(pGeometry->GetNumIndices(), m_uInstanceCount, 0, 0, m_uInstanceStart);
		}
	}

	UIWindow::Render(pCommandList, uFenceValue);
}