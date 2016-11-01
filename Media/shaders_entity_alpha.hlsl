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

struct VSInput
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD0;
	float3 tangent	: TANGENT;
};

struct PSInput
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 g_mWorldView;
	float4x4 g_mWorldViewProj;
	float4x4 g_mWorldViewProjInv;
};
PSInput VSMain(VSInput input)
{
	PSInput result;
	
	result.position = mul(float4(input.position, 1.0f), g_mWorldViewProj);
	result.uv = input.uv;
	
	return result;
}

Texture2D	g_txDiffuse	: register(t0);
Texture2D	g_txNormal	: register(t1);
SamplerState	g_sampler	: register(s0);

struct SPixelLink
{
	float4 color;
	float depth;
	uint next;
};
RWStructuredBuffer<SPixelLink> gPixelLinkBuffer : register(u1);
RWStructuredBuffer<uint> gStartOffsetBuffer : register(u2);

//float4 PSMain(PSInput input) : SV_TARGET
//{
//	return float4(1,1,1,1);
//
//	float3 diffuse = g_txDiffuse.Sample(g_sampler, input.uv).rgb;
//	float3 normal = g_txNormal.Sample(g_sampler, input.uv).rgb;
//	return float4(diffuse*normal,1.0f);
//}

float4 PSMain(PSInput input,float4 ScreenPos:SV_POSITION) : SV_TARGET
{
	uint uCounter = gPixelLinkBuffer.IncrementCounter();
	uint uOffset = ScreenPos.y*1280 + ScreenPos.x;

	//
	uint uOldCounter;
	InterlockedExchange(gStartOffsetBuffer[uOffset],uCounter,uOldCounter);

	gPixelLinkBuffer[uCounter].color = float4(0,0,1,0.25);
	gPixelLinkBuffer[uCounter].depth = input.position.z;
	gPixelLinkBuffer[uCounter].next = uOldCounter;
	discard;
	
	//return input.color;
	return g_txDiffuse.Sample(g_sampler, input.uv);
}
