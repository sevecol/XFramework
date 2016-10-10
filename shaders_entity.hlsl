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

cbuffer cb0 : register(b0)
{
	float4x4 g_mWorldViewProj;
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

struct PsOutput
{
	float4 color0 	: SV_TARGET0;
	float4 color1  	: SV_TARGET1;
	float4 color2  	: SV_TARGET2;
};

//float4 PSMain(PSInput input) : SV_TARGET
//{
//	return float4(1,1,1,1);
//
//	float3 diffuse = g_txDiffuse.Sample(g_sampler, input.uv).rgb;
//	float3 normal = g_txNormal.Sample(g_sampler, input.uv).rgb;
//	return float4(diffuse*normal,1.0f);
//}

PsOutput PSMain(PSInput input)
{
	PsOutput result;
	result.color0 = g_txDiffuse.Sample(g_sampler, input.uv);
	result.color1 = float4(1,1,1,1);
	result.color2 = float4(0,0,0,1);

	return result;
	//return g_texture.Sample(g_sampler, input.uv);
}
