
#include "XEntity.h"
#include "d3dx12.h"
#include "DXSampleHelper.h"
#include "Math\XMathSIMD.h"

extern XResourceThread *g_pResourceThread;
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);

struct sVertex
{
	Vector3f	m_vPos;
	Vector2f	m_vTex;
	UINT32		m_uIndex;
	Vector4f	m_vWeight;
};
sVertex *pData = nullptr;

namespace Entity
{
	UINT								uGpuCSUBase;
}
using namespace Entity;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XEntity::Init(ID3D12Device* pDevice)
{
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,2);
}

XEntity::XEntity():m_pTextureSet(nullptr),m_pShader(nullptr),m_pGeometry(nullptr){}
XEntity::~XEntity()
{
	XTextureSet::DeleteTextureSet(&m_pTextureSet);
	SAFE_DELETE(m_pShader);
	XGeometry::DeleteGeometry(&m_pGeometry);
}

//
/*
void XEntity::SetResourceCheck(UINT uResourceCheck)
{
}
void XEntity::IncreaseResourceComplate()
{
}

void XEntity::Update(UINT32 deltatime)
{
	//Instance::Update(deltatime);
	//m_SkeletonAnimateControl.Update(deltatime, m_pGeometry);
}

IntersectionResult XEntity::Update(const OptFrustum* const frustum, UINT32 deltatime)
{
	return IR_Inside;
}
*/
void XEntity::Render(ID3D12GraphicsCommandList* pCommandList, UINT64 uFenceValue)
{
	if ((pCommandList)&&(m_pShader)&&(m_pShader->GetPipelineState()))
	{
		if (m_pTextureSet)
		{
			pCommandList->SetGraphicsRootDescriptorTable(2, m_pTextureSet->GetSRVGpuHandle());
		}
		
		//
		pCommandList->SetPipelineState(m_pShader->GetPipelineState());
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->IASetVertexBuffers(0, 1, m_pGeometry->GetVertexBufferView());
		if (m_pGeometry->GetNumIndices())
		{
			pCommandList->IASetIndexBuffer(m_pGeometry->GetIndexBufferView());
			pCommandList->DrawIndexedInstanced(m_pGeometry->GetNumIndices(), 1, 0, 0, 0);
		}
		else
		{
			pCommandList->DrawInstanced(3, 1, 0, 0);
		}
	}

	//
	m_uFenceValue = uFenceValue;
}
XShader* XEntity::InitShader(LPCWSTR pFileName,LPCSTR pVSEntryPoint,LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[],UINT uInputElementCount, ESHADINGPATH eShadingPath)
{
	m_pShader = XShader::CreateShaderFromFile(pFileName, pVSEntryPoint, pVSTarget, pPSEntryPoint, pPSTarget, InputElementDescs, uInputElementCount, eShadingPath);
	return m_pShader;
}
XShader* XEntity::InitShader(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, UINT uRenderTargetCount,DXGI_FORMAT RenderTargetFormat[])
{
	m_pShader = XShader::CreateShaderFromFile(pFileName, pVSEntryPoint, pVSTarget, pPSEntryPoint, pPSTarget, InputElementDescs, uInputElementCount, uRenderTargetCount, RenderTargetFormat);
	return m_pShader;
}
/*
bool Entity::InitMaterial(LPCWSTR pName, UINT uWidth,UINT uHeight,UINT uPixelSize, CreateTextureFun pFun, UINT uParameter)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_pTexture = GetXEngine()->GetTextureManager()->CreateTexture(pName, uWidth, uHeight, uPixelSize, pFun, uParameter,this);
	return true;
}
*/
XTextureSet* XEntity::InitTexture(LPCWSTR pName,UINT uCount, LPCWSTR pFileName[], XTextureSet::eTextureFileType eFileType)
{
	m_pTextureSet = XTextureSet::CreateTextureSet(pName, uCount, pFileName, uGpuCSUBase, eFileType);
	return m_pTextureSet;
}

XTextureSet* XEntity::InitTexture(LPCWSTR pName, UINT uWidth,UINT uHeight, DXGI_FORMAT Format, UINT8 *pData, UINT uPixelSize)
{
	m_pTextureSet = XTextureSet::CreateTextureSet(pName, uGpuCSUBase, uWidth, uHeight, Format, pData, uPixelSize);
	return m_pTextureSet;
}

XGeometry* XEntity::InitGeometry(LPCWSTR pName, UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData)
{
/*
	// Create an upload heap for the constant buffers.
	ThrowIfFailed(g_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantBufferUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	ThrowIfFailed(m_pConstantBufferUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pConstantBufferUploadHeap->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(ConstantBuffer);
	g_pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), 3, g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
*/
	//
	m_pGeometry = XGeometry::CreateGeometry(pName,uVertexCount, uVertexStride, uIndexCount, uIndexFormat, pGeometryData);
	if (m_pGeometry)
	{
		//m_AxisAlignedBoundingBox.Add(m_pGeometry->m_vMin);
		//m_AxisAlignedBoundingBox.Add(m_pGeometry->m_vMax);
	}

	return m_pGeometry;
}
/*
bool Entity::InitSkeleton(LPCWSTR pFileName)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_SkeletonAnimateControl.m_pSkeleton = new Skeleton;//g_pXEngine->GetGeometryManager()->CreateGeometry(pName);
	if (m_SkeletonAnimateControl.m_pSkeleton)
	{
		GetXEngine()->GetResourceThread()->InsertSkeletonLoadTask(pFileName, dynamic_cast<Skeleton*>(m_SkeletonAnimateControl.m_pSkeleton), this);
	}

	return true;
}
bool Entity::InitAnimate(LPCWSTR pFileName)
{
	if (!GetXEngine())
	{
		return false;
	}

	m_SkeletonAnimateControl.m_pAnimate = new Animate;//g_pXEngine->GetGeometryManager()->CreateGeometry(pName);
	if (m_SkeletonAnimateControl.m_pAnimate)
	{
		GetXEngine()->GetResourceThread()->InsertAnimateLoadTask(pFileName, dynamic_cast<Animate*>(m_SkeletonAnimateControl.m_pAnimate), this);
	}

	return true;
}

void Entity::PlayAnimate(UINT32 uAnimateID)
{
	m_SkeletonAnimateControl.PlayAnimate(uAnimateID);
}
*/