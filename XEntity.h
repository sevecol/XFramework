
#pragma once

#include "Resource\XShader.h"
#include "Resource\XGeometry.h"

class XEntity
{
public:
	XShader							*m_pShader;
	XGeometry						*m_pGeometry;

	UINT64							m_uFenceValue;

	//
	ComPtr<ID3D12Resource>			m_pConstantBufferUploadHeap;
	UINT16							m_uIndex;

	struct ConstantBuffer
	{
		XMFLOAT4X4 mvp;				// Model-view-projection (MVP) matrix.
		FLOAT padding[48];
	};
	ConstantBuffer*					m_pConstantBuffers;

public:
	//
	XEntity();
	~XEntity();

	//
	virtual void SetResourceCheck(UINT uResourceCheck);
	virtual void IncreaseResourceComplate();

	//
	virtual void Update(UINT32 deltatime);
	//virtual IntersectionResult Update(const OptFrustum* const frustum, UINT32 deltatime);
	void Render(ID3D12GraphicsCommandList* pCommandList,UINT64 uFenceValue);

	//
	virtual bool InitShader(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount);
	//virtual bool InitMaterial(LPCWSTR pName, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter);
	//virtual bool InitMaterial(LPCWSTR pFileName, UINT uCount, LPCWSTR pDetailName[]);
	virtual bool InitGeometry(LPCWSTR pName, UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData, UINT uBoneCount = 0, UINT8* pBoneIndex = nullptr);
	//virtual bool InitSkeleton(LPCWSTR pFileName);
	//virtual bool InitAnimate(LPCWSTR pFileName);

	//virtual void PlayAnimate(UINT32 uAnimateID);
};