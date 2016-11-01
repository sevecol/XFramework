//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct SPixelLink
{
	float4 color;
	float depth;
	uint next;
};
RWStructuredBuffer<SPixelLink> gPixelLinkBuffer : register(u1);
RWStructuredBuffer<uint> gStartOffsetBuffer : register(u2);

PSInput VSMain(uint uInstanceID : SV_InstanceID, float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.color = color;
	result.uv = uv;

	return result;
}

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

#define MAX_PIXELS 	4

float4 PSMain(PSInput input,float4 ScreenPos:SV_POSITION) : SV_TARGET
{
	//return float4(0,0,1,1);

	uint uOffset = ScreenPos.y*1280 + ScreenPos.x;
	//uint uOffset = (input.uv.y * 719)*1280 + input.uv.x * 1279;
	uint uCounter = gStartOffsetBuffer[uOffset];

	SPixelLink SortedPixels[MAX_PIXELS];

	uint uNumPixels = 0;
	while (uCounter!=0xFFFFFFFF)
	{
		SortedPixels[uNumPixels  ].color = gPixelLinkBuffer[uCounter].color;
		SortedPixels[uNumPixels++].depth = gPixelLinkBuffer[uCounter].depth;

		uCounter = (uNumPixels>=MAX_PIXELS)?0xFFFFFFFF:gPixelLinkBuffer[uCounter].next;
	}

	if (uNumPixels==0)
		discard;

	// SortPixel
	if (uNumPixels>=2)
	{
		for (uint i=0;i<uNumPixels-1;i++)
		{
			for (uint j=0;j<uNumPixels-i-1;j++)
			{
				if (SortedPixels[j].depth>SortedPixels[j+1].depth)
				{
					SPixelLink temp;
					temp.color = SortedPixels[j].color;
					temp.depth = SortedPixels[j].depth;

					SortedPixels[j].color = SortedPixels[j+1].color;
					SortedPixels[j].depth = SortedPixels[j+1].depth;

					SortedPixels[j+1].color = temp.color;
					SortedPixels[j+1].depth = temp.depth;
				}
			}
		}
	}

	float4 fFinalColor = float4(0,0,0,0);
	for (int i=0;i<uNumPixels;i++)
	{
		fFinalColor.xyz = SortedPixels[i].color.xyz * SortedPixels[i].color.w + fFinalColor.xyz * (1-SortedPixels[i].color.w);
	}

	return fFinalColor;
}
