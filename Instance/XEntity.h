
#pragma once

#include "..\SceneGraph\XNode.h"
#include "..\Resource\XTexture.h"
#include "..\Resource\XShader.h"
#include "..\Resource\XGeometry.h"

class XEntity : public XNode
{
public:
	XTextureSet						*m_pTextureSet;
	XShader							*m_pShader;
	XGeometry						*m_pGeometry;

	UINT64							m_uFenceValue;

	struct ConstantBuffer
	{
		XMFLOAT4X4	m;				// Model-view-projection (MVP) matrix.
		UINT		uIndex;			//
		FLOAT padding[47];
	};
	ConstantBuffer*					m_pConstantBuffers;
	ComPtr<ID3D12Resource>			m_pConstantUploadHeap;

public:
	//
	XEntity();
	~XEntity();

	static void Init(ID3D12Device* pDevice);

	//
	//virtual void SetResourceCheck(UINT uResourceCheck);
	//virtual void IncreaseResourceComplate();

	//
	//virtual void Update(UINT32 deltatime);
	//virtual IntersectionResult Update(const OptFrustum* const frustum, UINT32 deltatime);
	virtual void Render(ID3D12GraphicsCommandList* pCommandList,UINT64 uFenceValue);
	virtual void Update();

	//
	XShader* InitShader(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, UINT uRenderTargetCount, DXGI_FORMAT RenderTargetFormat[]);
	XShader* InitShader(LPCWSTR pFileName, LPCSTR pVSEntryPoint, LPCSTR pVSTarget, LPCSTR pPSEntryPoint, LPCSTR pPSTarget, D3D12_INPUT_ELEMENT_DESC InputElementDescs[], UINT uInputElementCount, ESHADINGPATH eShadingPath = ESHADINGPATH_FORWORD);
	//virtual bool InitMaterial(LPCWSTR pName, UINT uWidth, UINT uHeight, UINT uPixelSize, CreateTextureFun pFun, UINT uParameter);
	XTextureSet* InitTexture(LPCWSTR pName, UINT uCount, LPCWSTR pFileName[]);
	XTextureSet* InitTexture(LPCWSTR pName, UINT uWidth, UINT uHeight, DXGI_FORMAT Format, UINT8 *pData, UINT uPixelSize);
	XGeometry* InitGeometry(LPCWSTR pName, UINT uVertexCount, UINT uVertexStride, UINT uIndexCount, UINT uIndexFormat, UINT8* pGeometryData);
	//virtual bool InitSkeleton(LPCWSTR pFileName);
	//virtual bool InitAnimate(LPCWSTR pFileName);

	//virtual void PlayAnimate(UINT32 uAnimateID);
};