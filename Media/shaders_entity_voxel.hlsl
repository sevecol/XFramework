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
	float4 position		: SV_POSITION;
        float3 positionW	: positionW;		// World space position
    	float3 normal      	: normalW;
	float3 tangent		: tangentW;
	float2 uv		: TEXCOORD0;
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 mView;
	float4x4 mViewProj;
	float4x4 mViewProjInv;
};
cbuffer InstanceBuffer : register(b1)
{
	float4x4 mWorld;
};

PSInput VSMain(VSInput input)
{
	PSInput result;

	float3 position = mul(float4(input.position.x,input.position.y,input.position.z,1.0f), mWorld).xyz;
	float3 normal = float3(input.normal.x,input.normal.y,input.normal.z);
	
	result.position = mul(float4(position, 1.0f), mViewProj);

	result.positionW 	= position.xyz;
	result.normal  		= normal.xyz;
	result.tangent		= input.tangent;

	result.uv = input.uv;
	
	return result;
}

Texture2D	g_txDiffuse	: register(t0);
Texture2D	g_txNormal	: register(t1);
Texture2D	g_txMask	: register(t2);
SamplerState	g_sampler	: register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(1,1,1,1);
}
