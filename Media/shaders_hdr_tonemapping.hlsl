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

PSInput VSMain(uint uInstanceID : SV_InstanceID, float4 position : POSITION, float4 color : COLOR, float2 uv : TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.color = color;
	result.uv = uv;

	return result;
}

cbuffer FrameBuffer : register(b1)
{
	uint2 uDispatch;					// x and y dimensions of the Dispatch call
	uint2 uScreen;						// Size of the input Texture2D
	float fValue;
};

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);
RWStructuredBuffer<float>		g_result 		: register(u1);

static const float4 	c_luminance		= float4(.299, .587, .114, 0);
static const float	c_middleGray		= 0.72f;
static const float	c_luminanceWhite	= 1.5f;
static const float	c_brightThreshold	= 0.5f;

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 color = g_texture0.Sample(g_sampler, input.uv);
	
	return color;
	//if (input.uv.x>0.5f)
	//	return color;
	
	float fluminance =  g_result[0]/(uScreen.x*uScreen.y);

	// Tone mapping
	color.rgb *= c_middleGray / (fluminance + 0.001f);
	color.rgb *= (1.f + color.rgb / c_luminanceWhite);
	color.rgb /= (1.f + color.rgb);

	//color.rgb += 0.6f * bloom.rgb;
	color.a    = 1.f;

	return color;
}
