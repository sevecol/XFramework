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
	result.uv = (float3)normalize( mul( input.position, g_mWorldViewProjInv ) );
	
	return result;
}

TextureCube	g_Environment	: register( t0 );
SamplerState 	g_sampler	: register( s0 );

struct PsOutput
{
	float4 color0 	: SV_TARGET0;
	float4 color1  	: SV_TARGET1;
	float4 color2  	: SV_TARGET2;
};

PsOutput PSMain(PSInput input)
{
	PsOutput result;
	result.color0 = float4(input.uv,1.0f);//float4(1,1,1,1);//g_Environment.Sample( g_sampler, input.uv );
	result.color0 = g_Environment.Sample( g_sampler, input.uv );
	result.color1 = g_Environment.Sample( g_sampler, input.uv );
	result.color2 = g_Environment.Sample( g_sampler, input.uv );

	return result;
}
