
#include "XSMAA.h"
#include "..\..\DXSampleHelper.h"

#include "..\..\Resource\XTexture.h"
#include "..\..\Resource\XShader.h"

extern XEngine *g_pEngine;
extern UINT GetHandleHeapStart(XEngine::XDescriptorHeapType eType, UINT uCount);

namespace SMAA
{
	UINT										uGpuCSUBase;
	//UINT										uRenderTargetBase, uGpuCSUBase;
	//UINT										uDispatchX, uDispatchY;
	//XRenderTarget								*pRenderTargets[DEFERREDSHADING_RENDERTARGET_COUNT] = { nullptr,nullptr,nullptr };
	XGraphicShader								*pShadingShader = nullptr;
	XTextureSet									*pTexture		= nullptr;
	ID3D12Resource								*pCpuTexture	= nullptr;
}
using namespace SMAA;

UINT8 guType[1024][1024];
UINT8 guMask[1024][1024];
bool InitSMAA(ID3D12Device* pDevice, UINT uWidth, UINT uHeight)
{
	uGpuCSUBase = GetHandleHeapStart(XEngine::XDESCRIPTORHEAPTYPE_GCSU, 1);

	// Texture
	LPCWSTR lpTextureFileName[] = {L"Media\\smaa64.bmp"};
	pTexture = XTextureSetManager::CreateTextureSet(L"SMAATexture", 1, lpTextureFileName, uGpuCSUBase);
	D3D12_RESOURCE_DESC textureDesc = pTexture->GetResource(0)->GetDesc();

	ThrowIfFailed(g_pEngine->m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(pTexture->GetSize(0)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pCpuTexture)));

	//
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShadingShader = XGraphicShaderManager::CreateGraphicShaderFromFile(L"Media\\SMAA.hlsl", "VSMain", "vs_5_0", "PSMain", "ps_5_0", inputElementDescs, 3);

	return true;
}
void CleanSMAA()
{
	XTextureSetManager::DelResource(&pTexture);
	XGraphicShaderManager::DelResource(&pShadingShader);

	//
	SAFE_RELEASE(pCpuTexture);
}

extern void RenderFullScreen(ID3D12GraphicsCommandList *pCommandList, XGraphicShader *pShader, XTextureSet *pTexture = nullptr);
void SMAA_Render(ID3D12GraphicsCommandList* pCommandList)
{
	//
	static bool bFirst = true;
	if (bFirst)
	{
		bFirst = false;

		//
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[8];
		uint64_t row_sizes_in_bytes[8];
		uint32_t num_rows[8];

		D3D12_RESOURCE_DESC textureDesc = pTexture->GetResource(0)->GetDesc();

		uint64_t required_size = 0;
		g_pEngine->m_pDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, &layouts[0], &num_rows[0], &row_sizes_in_bytes[0], &required_size);

		// Copy To CpuTexture
		//pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetResource(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = pTexture->GetResource(0);
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = layouts[0];

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = pCpuTexture;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;
		//pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		//
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetResource(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

		//
		UINT8 *pSData = nullptr;
		CD3DX12_RANGE readRange(0, pTexture->GetSize(0));
		pCpuTexture->Map(0, &readRange, reinterpret_cast<void**>(&pSData));

		float fValue = 0.5f;

		UINT8 *pData = pSData;
		pData += layouts[0].Footprint.RowPitch;
		for (unsigned int i = 1;i <=(layouts[0].Footprint.Height - 1);++i)
		{
			for (unsigned int j = 0;j < (layouts[0].Footprint.Width - 1);++j)
			{
				float fRC = pData[j * 4 + 0] / 255.0f;
				float fGC = pData[j * 4 + 1] / 255.0f;
				float fBC = pData[j * 4 + 2] / 255.0f;

				float fRR = pData[(j + 1) * 4 + 0] / 255.0f;
				float fGR = pData[(j + 1) * 4 + 1] / 255.0f;
				float fBR = pData[(j + 1) * 4 + 2] / 255.0f;

				UINT8 *pDataU = pData - layouts[0].Footprint.RowPitch;
				float fRT = pDataU[j * 4 + 0] / 255.0f;
				float fGT = pDataU[j * 4 + 1] / 255.0f;
				float fBT = pDataU[j * 4 + 2] / 255.0f;

				float fLC = fRC * 0.2126f + fGC * 0.7152f + fBC * 0.0722f;
				float fLR = fRR * 0.2126f + fGR * 0.7152f + fBR * 0.0722f;
				float fLT = fRT * 0.2126f + fGT * 0.7152f + fBT * 0.0722f;

				UINT8 uType = 0;
				if (fabs(fLR - fLC) > fValue)
				{
					uType |= 1;
				}
				if (fabs(fLT - fLC) > fValue)
				{
					uType |= 2;
				}
				guType[i][j] = uType;
			}

			pData += layouts[0].Footprint.RowPitch;
		}
		pData = pSData;
		for (unsigned int i = 0;i <layouts[0].Footprint.Height;++i)
		{
			for (unsigned int j = 0;j <layouts[0].Footprint.Width;++j)
			{
				if (guMask[i][j] == 1)
					continue;

				if ((guType[i][j] == 2)||(guType[i][j] == 3))
				{
					bool bFind = false;
					unsigned int r = 0;
					bool rup = true;
					for (unsigned int k = 1;k < 3;++k)
					{
						if ((j + k) < layouts[0].Footprint.Width )
						{
							switch (guType[i][j + k])
							{
							case 0:
							case 1:
								{
									r = k - 1;
									bFind = true;
									if (guType[i][j] == 2)
									{
										rup = true;
									}
									else
									{
										rup = false;
									}
								}
								break;
							//case 1:
							//	break;
							case 2:
								break;
							case 3:
								r = k;
								rup = false;
								bFind = true;
								break;
							}

							//
							if (bFind)
							{
								break;
							}
						}
					}
					if (!bFind)
					{
						r = 5;
					}

					bFind = false;
					unsigned int l = 0;
					bool lup = true;
					for (unsigned int k = 1;k < 3;++k)
					{
						if (j >= k)
						{
							switch (guType[i][j - k])
							{
							case 0:
								l = k-1;
								lup = true;
								bFind = true;
								break;
							case 1:
								l = k-1;
								lup = false;
								bFind = true;
								break;
							case 2:
								break;
							case 3:
								break;
							}

							//
							if (bFind)
							{
								break;
							}
						}
					}
					if (!bFind)
					{
						l = 5;
					}

					//
					unsigned int uLength = l + r;
					if (uLength <=2)
					{
						//
						UINT8 *pDataU = pData - layouts[0].Footprint.RowPitch;
						UINT8 *pDataB = pData + layouts[0].Footprint.RowPitch;

						//
						UINT8 uR0, uG0, uB0, uA0;
						UINT8 uR1, uG1, uB1, uA1;

						//
						switch (uLength)
						{
						case 0:
							{
								unsigned int uBase = j;
								guMask[i][uBase] = 1;

								if (lup != rup)
								{
									uR0 = pDataU[uBase * 4 + 0] * 7.0f / 8.0f + pData[uBase * 4 + 0] * 1.0f / 8.0f;
									uG0 = pDataU[uBase * 4 + 1] * 7.0f / 8.0f + pData[uBase * 4 + 1] * 1.0f / 8.0f;
									uB0 = pDataU[uBase * 4 + 2] * 7.0f / 8.0f + pData[uBase * 4 + 2] * 1.0f / 8.0f;
									uA0 = pDataU[uBase * 4 + 3] * 7.0f / 8.0f + pData[uBase * 4 + 3] * 1.0f / 8.0f;
									uR1 = pDataU[uBase * 4 + 0] * 1.0f / 8.0f + pData[uBase * 4 + 0] * 7.0f / 8.0f;
									uG1 = pDataU[uBase * 4 + 1] * 1.0f / 8.0f + pData[uBase * 4 + 1] * 7.0f / 8.0f;
									uB1 = pDataU[uBase * 4 + 2] * 1.0f / 8.0f + pData[uBase * 4 + 2] * 7.0f / 8.0f;
									uA1 = pDataU[uBase * 4 + 3] * 1.0f / 8.0f + pData[uBase * 4 + 3] * 7.0f / 8.0f;

									// Z1,Z2
									pDataU[uBase * 4 + 0] = uR0;
									pDataU[uBase * 4 + 1] = uG0;
									pDataU[uBase * 4 + 2] = uB0;
									pDataU[uBase * 4 + 3] = uA0;
									pData [uBase * 4 + 0] = uR1;
									pData [uBase * 4 + 1] = uG1;
									pData [uBase * 4 + 2] = uB1;
									pData [uBase * 4 + 3] = uA1;
								}
								else
								{
									// U
									if (lup)
									{
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										// U1
										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
									}
									else
									{
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 4.0f + pData[uBase * 4 + 0] * 3.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 4.0f + pData[uBase * 4 + 1] * 3.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 4.0f + pData[uBase * 4 + 2] * 3.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 4.0f + pData[uBase * 4 + 3] * 3.0f / 4.0f;

										// U2
										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;
									}
								}
							}
							break;
						case 1:
							{
								unsigned int uBase = j - l;
								guMask[i][uBase+0] = 1;
								guMask[i][uBase+1] = 1;

								if (lup != rup)
								{
									// Z
									if (lup)
									{
										// Z1
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 4.0f + pData[uBase * 4 + 0] * 3.0f / 2.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 4.0f + pData[uBase * 4 + 1] * 3.0f / 2.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 4.0f + pData[uBase * 4 + 2] * 3.0f / 2.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 4.0f + pData[uBase * 4 + 3] * 3.0f / 2.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;
									}
									else
									{
										// Z2
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 4.0f + pData[uBase * 4 + 0] * 3.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 4.0f + pData[uBase * 4 + 1] * 3.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 4.0f + pData[uBase * 4 + 2] * 3.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 4.0f + pData[uBase * 4 + 3] * 3.0f / 4.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
									}
								}
								else
								{
									// U 
									if (lup)
									{
										// U1
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
									}
									else
									{
										// U2
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 3.0f / 4.0f + pData[uBase * 4 + 0] * 1.0f / 4.0f;
										uG0 = pDataU[uBase * 4 + 1] * 3.0f / 4.0f + pData[uBase * 4 + 1] * 1.0f / 4.0f;
										uB0 = pDataU[uBase * 4 + 2] * 3.0f / 4.0f + pData[uBase * 4 + 2] * 1.0f / 4.0f;
										uA0 = pDataU[uBase * 4 + 3] * 3.0f / 4.0f + pData[uBase * 4 + 3] * 1.0f / 4.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;
									}
								}
							}
							break;
						case 2:
							{
								unsigned int uBase = j - l;
								guMask[i][uBase+0] = 1;
								guMask[i][uBase+1] = 1;
								guMask[i][uBase+2] = 1;

								if (lup != rup)
								{
									// Z
									if (lup)
									{
										// Z1
										uR0 = pDataU[uBase * 4 + 0] * 2.0f / 3.0f + pData[uBase * 4 + 0] * 1.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 2.0f / 3.0f + pData[uBase * 4 + 1] * 1.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 2.0f / 3.0f + pData[uBase * 4 + 2] * 1.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 2.0f / 3.0f + pData[uBase * 4 + 3] * 1.0f / 3.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
										
										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 23.0f / 24.0f + pData[uBase * 4 + 0] * 1.0f / 24.0f;
										uG0 = pDataU[uBase * 4 + 1] * 23.0f / 24.0f + pData[uBase * 4 + 1] * 1.0f / 24.0f;
										uB0 = pDataU[uBase * 4 + 2] * 23.0f / 24.0f + pData[uBase * 4 + 2] * 1.0f / 24.0f;
										uA0 = pDataU[uBase * 4 + 3] * 23.0f / 24.0f + pData[uBase * 4 + 3] * 1.0f / 24.0f;
										uR1 = pData[uBase * 4 + 0] * 23.0f / 24.0f + pDataU[uBase * 4 + 0] * 1.0f / 24.0f;
										uG1 = pData[uBase * 4 + 1] * 23.0f / 24.0f + pDataU[uBase * 4 + 1] * 1.0f / 24.0f;
										uB1 = pData[uBase * 4 + 2] * 23.0f / 24.0f + pDataU[uBase * 4 + 2] * 1.0f / 24.0f;
										uA1 = pData[uBase * 4 + 3] * 23.0f / 24.0f + pDataU[uBase * 4 + 3] * 1.0f / 24.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
										pData [uBase * 4 + 0] = uR1;
										pData [uBase * 4 + 1] = uG1;
										pData [uBase * 4 + 2] = uB1;
										pData [uBase * 4 + 3] = uA1;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 3.0f + pData[uBase * 4 + 0] * 2.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 3.0f + pData[uBase * 4 + 1] * 2.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 3.0f + pData[uBase * 4 + 2] * 2.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 3.0f + pData[uBase * 4 + 3] * 2.0f / 3.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;
									}
									else
									{
										// Z2
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 3.0f + pData[uBase * 4 + 0] * 2.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 3.0f + pData[uBase * 4 + 1] * 2.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 3.0f + pData[uBase * 4 + 2] * 2.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 3.0f + pData[uBase * 4 + 3] * 2.0f / 3.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 23.0f / 24.0f + pData[uBase * 4 + 0] * 1.0f / 24.0f;
										uG0 = pDataU[uBase * 4 + 1] * 23.0f / 24.0f + pData[uBase * 4 + 1] * 1.0f / 24.0f;
										uB0 = pDataU[uBase * 4 + 2] * 23.0f / 24.0f + pData[uBase * 4 + 2] * 1.0f / 24.0f;
										uA0 = pDataU[uBase * 4 + 3] * 23.0f / 24.0f + pData[uBase * 4 + 3] * 1.0f / 24.0f;
										uR1 = pData[uBase * 4 + 0] * 23.0f / 24.0f + pDataU[uBase * 4 + 0] * 1.0f / 24.0f;
										uG1 = pData[uBase * 4 + 1] * 23.0f / 24.0f + pDataU[uBase * 4 + 1] * 1.0f / 24.0f;
										uB1 = pData[uBase * 4 + 2] * 23.0f / 24.0f + pDataU[uBase * 4 + 2] * 1.0f / 24.0f;
										uA1 = pData[uBase * 4 + 3] * 23.0f / 24.0f + pDataU[uBase * 4 + 3] * 1.0f / 24.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
										pData [uBase * 4 + 0] = uR1;
										pData [uBase * 4 + 1] = uG1;
										pData [uBase * 4 + 2] = uB1;
										pData [uBase * 4 + 3] = uA1;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 2.0f / 3.0f + pData[uBase * 4 + 0] * 1.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 2.0f / 3.0f + pData[uBase * 4 + 1] * 1.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 2.0f / 3.0f + pData[uBase * 4 + 2] * 1.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 2.0f / 3.0f + pData[uBase * 4 + 3] * 1.0f / 3.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
									}
								}
								else
								{
									// U
									if (lup)
									{
										// U1
										uR0 = pDataU[uBase * 4 + 0] * 2.0f / 3.0f + pData[uBase * 4 + 0] * 1.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 2.0f / 3.0f + pData[uBase * 4 + 1] * 1.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 2.0f / 3.0f + pData[uBase * 4 + 2] * 1.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 2.0f / 3.0f + pData[uBase * 4 + 3] * 1.0f / 3.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 11.0f / 12.0f + pData[uBase * 4 + 0] * 1.0f / 12.0f;
										uG0 = pDataU[uBase * 4 + 1] * 11.0f / 12.0f + pData[uBase * 4 + 1] * 1.0f / 12.0f;
										uB0 = pDataU[uBase * 4 + 2] * 11.0f / 12.0f + pData[uBase * 4 + 2] * 1.0f / 12.0f;
										uA0 = pDataU[uBase * 4 + 3] * 11.0f / 12.0f + pData[uBase * 4 + 3] * 1.0f / 12.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 2.0f / 3.0f + pData[uBase * 4 + 0] * 1.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 2.0f / 3.0f + pData[uBase * 4 + 1] * 1.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 2.0f / 3.0f + pData[uBase * 4 + 2] * 1.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 2.0f / 3.0f + pData[uBase * 4 + 3] * 1.0f / 3.0f;

										pDataU[uBase * 4 + 0] = uR0;
										pDataU[uBase * 4 + 1] = uG0;
										pDataU[uBase * 4 + 2] = uB0;
										pDataU[uBase * 4 + 3] = uA0;
									}
									else
									{
										// U2
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 3.0f + pData[uBase * 4 + 0] * 2.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 3.0f + pData[uBase * 4 + 1] * 2.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 3.0f + pData[uBase * 4 + 2] * 2.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 3.0f + pData[uBase * 4 + 3] * 2.0f / 3.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 12.0f + pData[uBase * 4 + 0] * 11.0f / 12.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 12.0f + pData[uBase * 4 + 1] * 11.0f / 12.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 12.0f + pData[uBase * 4 + 2] * 11.0f / 12.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 12.0f + pData[uBase * 4 + 3] * 11.0f / 12.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;

										uBase++;
										uR0 = pDataU[uBase * 4 + 0] * 1.0f / 3.0f + pData[uBase * 4 + 0] * 2.0f / 3.0f;
										uG0 = pDataU[uBase * 4 + 1] * 1.0f / 3.0f + pData[uBase * 4 + 1] * 2.0f / 3.0f;
										uB0 = pDataU[uBase * 4 + 2] * 1.0f / 3.0f + pData[uBase * 4 + 2] * 2.0f / 3.0f;
										uA0 = pDataU[uBase * 4 + 3] * 1.0f / 3.0f + pData[uBase * 4 + 3] * 2.0f / 3.0f;

										pData[uBase * 4 + 0] = uR0;
										pData[uBase * 4 + 1] = uG0;
										pData[uBase * 4 + 2] = uB0;
										pData[uBase * 4 + 3] = uA0;
									}
								}
							}
							break;
						}
					}
				}
			}

			pData += layouts[0].Footprint.RowPitch;
		}
		CD3DX12_RANGE writeRange(0, pTexture->GetSize(0));
		pCpuTexture->Unmap(0, &writeRange);

		//
		src.pResource = pCpuTexture;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = layouts[0];

		dst.pResource = pTexture->GetResource(0);
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;
		pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		//
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetResource(0), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	//
	//RenderFullScreen(pCommandList, pShadingShader, pTexture);
}