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

struct GSInput
{
	float3 position		: POSITION;
};

struct PSInput
{
	float4 position		: SV_POSITION;
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

GSInput VSMain(VSInput input)
{
	GSInput result;
	result.position = input.position;
	
	return result;
}

Texture2D	g_txFront	: register(t0);
Texture2D	g_txTop		: register(t1);
Texture2D	g_txLeft	: register(t2);

[maxvertexcount(24)]
void GSMain(point GSInput input[1], inout TriangleStream<PSInput> triStream)
{
	PSInput result;
	
	//
	uint3 location = uint3(input[0].position.x+7.5f,input[0].position.y+7.5f, 0);
	float4 color = g_txFront.Load(location);

	if (color.r<0.9f)
	{
		for(int i=0;i<4;i++)
		{
			result.position = float4(0,0,0,1);
			triStream.Append(result);
		}
		return;
	}

	location = uint3(input[0].position.x+7.5f,input[0].position.z+7.5f, 0);
	color = g_txTop.Load(location);

	if (color.r<0.9f)
	{
		for(int i=0;i<4;i++)
		{
			result.position = float4(0,0,0,1);
			triStream.Append(result);
		}
		return;
	}

	location = uint3(input[0].position.z+7.5f,input[0].position.y+7.5f, 0);
	color = g_txLeft.Load(location);

	if (color.r<0.9f)
	{
		for(int i=0;i<4;i++)
		{
			result.position = float4(0,0,0,1);
			triStream.Append(result);
		}
		return;
	}

	//
	float3 offset[4];	

	// Font
	offset[0] = float3(-1, 1,-1);
	offset[1] = float3(-1,-1,-1);
	offset[2] = float3( 1, 1,-1);
	offset[3] = float3( 1,-1,-1);

	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();

	// Back
	offset[0] = float3(-1, 1,1);
	offset[1] = float3(-1,-1,1);
	offset[2] = float3( 1, 1,1);
	offset[3] = float3( 1,-1,1);
	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();

	// Top
	offset[0] = float3(-1, 1, 1);
	offset[1] = float3(-1, 1,-1);
	offset[2] = float3( 1, 1, 1);
	offset[3] = float3( 1, 1,-1);
	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();

	// Bottom
	offset[0] = float3(-1,-1, 1);
	offset[1] = float3(-1,-1,-1);
	offset[2] = float3( 1,-1, 1);
	offset[3] = float3( 1,-1,-1);
	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();

	// Left
	offset[0] = float3(-1, 1, 1);
	offset[1] = float3(-1,-1, 1);
	offset[2] = float3(-1, 1,-1);
	offset[3] = float3(-1,-1,-1);
	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();

	// Right
	offset[0] = float3( 1, 1, 1);
	offset[1] = float3( 1,-1, 1);
	offset[2] = float3( 1, 1,-1);
	offset[3] = float3( 1,-1,-1);
	for(int i=0;i<4;i++)
	{
		float3 position = input[0].position + offset[i];
		result.position = mul(float4(position, 1.0f), mViewProj);

		triStream.Append(result);
	}
	triStream.RestartStrip();
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(1,1,0,1);
}
