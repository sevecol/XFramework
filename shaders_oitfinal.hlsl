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

cbuffer cb0 : register(b0)
{
	float4x4 g_mWorldViewProj;
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
Texture2D g_texture1 : register(t1);
Texture2D g_texture2 : register(t2);
SamplerState g_sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
	uint uOffset = (input.uv.y * 719)*1280 + input.uv.x * 1279;
	return gPixelLinkBuffer[uOffset].color;
	return g_texture0.Sample(g_sampler, input.uv);
}
