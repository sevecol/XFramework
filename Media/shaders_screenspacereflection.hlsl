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
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float2 uv		: TEXCOORD0;
	float3 tangent		: TANGENT;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
cbuffer FrameBuffer : register(b0)
{
	float4x4 mView;
	float4x4 mViewProj;
	float4x4 mViewProjInv;
};

PSInput VSMain(VSInput input)
{
	PSInput result;

	float3 position = input.position * 10.0f;	
	result.position = mul(float4(position, 1.0f), mViewProj);
	result.uv = input.uv;

	return result;
}

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 color = g_texture0.Sample(g_sampler, input.uv);
	return float4(1,1,1,1);
}
