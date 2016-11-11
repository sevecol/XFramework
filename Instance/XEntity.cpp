
#include "XEntity.h"
#include "..\d3dx12.h"
#include "..\DXSampleHelper.h"
#include "..\Math\XMathSIMD.h"

extern XEngine *g_pEngine;
extern XResourceThread *g_pResourceThread;
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);
extern D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(XEngine::XDescriptorHeapType eType, UINT uIndex);

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
	UINT								uGpuCSUOffset;
}
using namespace Entity;

struct ShaderRenderTarget
{
	UINT			uCount;
	DXGI_FORMAT		Format[3];
};
ShaderRenderTarget gShaderRenderTarget[ERENDERPATH_COUNT] =
{
	{ 1, DXGI_FORMAT_R16G16B16A16_FLOAT },																	//ERENDERPATH_SHADOWMAP
	{ 3, DXGI_FORMAT_R16G16B16A16_FLOAT ,DXGI_FORMAT_R16G16B16A16_FLOAT ,DXGI_FORMAT_R16G16B16A16_FLOAT },	//ERENDERPATH_GEOMETRY
	{ 1, DXGI_FORMAT_R32G32B32A32_FLOAT },																	//ERENDERPATH_ALPHABLEND
	{ 1, DXGI_FORMAT_R32G32B32A32_FLOAT },																	//ERENDERPATH_POST
	{ 1, DXGI_FORMAT_R8G8B8A8_UNORM },																		//ERENDERPATH_VOXEL
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void XEntity::Init(ID3D12Device* pDevice)
{
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU,32);
	uGpuCSUOffset = 0;
}

XEntity::XEntity():m_pTextureSet(nullptr),m_pGeometry(nullptr)
{
	for (unsigned int i = 0;i < ERENDERPATH_COUNT;++i)
	{
		m_pShader[i] = nullptr;
	}

	// Create an upload heap for the constant buffers.
	ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pConstantUploadHeap)));

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_pConstantUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_pConstantBuffers)));

	//
	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantDesc = {};
	ConstantDesc.BufferLocation = m_pConstantUploadHeap->GetGPUVirtualAddress();
	ConstantDesc.SizeInBytes = sizeof(ConstantBuffer);
	g_pEngine->m_pDevice->CreateConstantBufferView(&ConstantDesc, GetCpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, uGpuCSUBase+uGpuCSUOffset));
	m_pConstantBuffers->uIndex = uGpuCSUBase + uGpuCSUOffset;
	uGpuCSUOffset++;
}
XEntity::~XEntity()
{
	m_pConstantUploadHeap->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;

	XTextureSetManager::DelResource(&m_pTextureSet);
	for (unsigned int i = 0;i < ERENDERPATH_COUNT;++i)
	{
		XGraphicShaderManager::DelResource(&m_pShader[i]);
	}
	
	XGeometryManager::DelResource(&m_pGeometry);
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
void XEntity::Render(ID3D12GraphicsCommandList* pCommandList, eRenderPath ePath, UINT64 uFenceValue)
{
	XGraphicShader *pShader = m_pShader[ePath];
	if (!pShader)
	{
		return;
	}

	//
	if ((pCommandList)&&(m_pShader)&&(pShader->GetPipelineState()))
	{
		if (m_pTextureSet)
		{
			pCommandList->SetGraphicsRootDescriptorTable(GRDT_SRV_TEXTURE, m_pTextureSet->GetSRVGpuHandle());
		}
		pCommandList->SetGraphicsRootDescriptorTable(GRDT_CBV_INSTANCEBUFFER, GetGpuDescriptorHandle(XEngine::XDESCRIPTORHEAPTYPE_GCSU, m_pConstantBuffers->uIndex));
		
		//
		pCommandList->SetPipelineState(pShader->GetPipelineState());
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
void XEntity::Update()
{
	XMMATRIX mModel, mRotationX, mRotationY, mRotationZ, mScale;
	XMFLOAT4X4 m;

	float fx, fy, fz;
	GetPos(fx, fy, fz);
	mModel = XMMatrixTranslation(fx, fy, fz);

	GetRotation(fx, fy, fz);
	mRotationX = XMMatrixRotationX(fx);
	mRotationY = XMMatrixRotationX(fy);
	mRotationZ = XMMatrixRotationX(fz);

	float fscale = GetScale();
	mScale = XMMatrixScaling(fscale, fscale, fscale);

	//
	XMMATRIX temp = XMMatrixTranspose(mScale*mRotationX*mRotationY*mRotationZ*mModel);
	XMStoreFloat4x4(&m, temp);
	m_pConstantBuffers->m = m;
}

XGraphicShader* XEntity::InitGraphicShader(eRenderPath ePath, LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_INPUT_ELEMENT_DESC InputElementDescs[],UINT uInputElementCount)
{
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_pShader[ePath] = XGraphicShaderManager::CreateGraphicShaderFromFile(pFileName, rGraphicShaderInfo, depthStencilDesc, InputElementDescs, uInputElementCount, gShaderRenderTarget[ePath].uCount, gShaderRenderTarget[ePath].Format);
	return m_pShader[ePath];
}
XGraphicShader* XEntity::InitGraphicShader(eRenderPath ePath, LPCWSTR pFileName, XGraphicShaderInfo& rGraphicShaderInfo, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, UINT uRenderTargetCount,DXGI_FORMAT RenderTargetFormat[])
{
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	m_pShader[ePath] = XGraphicShaderManager::CreateGraphicShaderFromFile(pFileName, rGraphicShaderInfo, depthStencilDesc, InputElementDescs, uInputElementCount, uRenderTargetCount, RenderTargetFormat);
	return m_pShader[ePath];
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
XTextureSet* XEntity::InitTexture(LPCWSTR pName,UINT uCount, LPCWSTR pFileName[])
{
	m_pTextureSet = XTextureSetManager::CreateTextureSet(pName, uCount, pFileName, uGpuCSUBase+ uGpuCSUOffset);
	uGpuCSUOffset += uCount;
	return m_pTextureSet;
}

XTextureSet* XEntity::InitTexture(LPCWSTR pName, UINT uWidth,UINT uHeight, DXGI_FORMAT Format, UINT8 *pData, UINT uPixelSize)
{
	m_pTextureSet = XTextureSetManager::CreateTextureSet(pName, uGpuCSUBase+ uGpuCSUOffset, uWidth, uHeight, Format, pData, uPixelSize);
	uGpuCSUOffset += 1;
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
	m_pGeometry = XGeometryManager::CreateGeometry(pName,uVertexCount, uVertexStride, uIndexCount, uIndexFormat, pGeometryData);
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