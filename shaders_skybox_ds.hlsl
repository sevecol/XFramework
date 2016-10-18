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
	float4 position	: POSITION;
	float4 color	: COLOR;
	float2 uv	: TEXCOORD0;
};

struct PSInput
{
	float4 position	: SV_POSITION;
	float3 uv	: TEXCOORD0;
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
	
	result.position = input.position;
	result.position.z = 1.0f;
	result.uv = (float3)normalize( mul( input.position, g_mWorldViewProjInv ) );
	
	return result;
}

TextureCube	g_Environment	: register( t0 );
SamplerState 	g_sampler	: register( s0 );

float4 PSMain(PSInput input) : SV_TARGET
{
	return g_Environment.Sample( g_sampler, input.uv );
}
