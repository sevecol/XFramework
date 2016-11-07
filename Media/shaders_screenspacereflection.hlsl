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
	float4 position 	: SV_POSITION;
        float3 positionW	: positionW;		// World space position
    	float3 normalW      	: normalW;
	float2 uv 		: TEXCOORD;
};
cbuffer FrameBuffer : register(b0)
{
	float4x4 mView;
	float4x4 mViewProj;
	float4x4 mViewProjInv;
	float4   vEyePos;
	float4	 vCameraNF;
};

PSInput VSMain(VSInput input)
{
	PSInput result;

	float3 position = input.position * 60.0f;
	position.y -= 8.0f;
	result.position = mul(float4(position, 1.0f), mViewProj);

	result.positionW = position;
	result.normalW = input.normal;

	result.uv = input.uv;

	return result;
}

Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
Texture2D g_texture2 : register(t2);
TextureCube g_EnvironmentLight : register(t3);
Texture2D g_texture5 : register(t5);
SamplerState g_sampler : register(s0);

//RWStructuredBuffer<float>		g_Result  : register(u1);

struct PsOutput
{
	float4 color 	: SV_TARGET;
	float depth 	: SV_DEPTH;
};

PsOutput PSMain(PSInput input)
{
	PsOutput result;

	float3 p = input.positionW;//float3(0.0,-8.0f,60.0f);//

	float3 vToPoint = normalize(p - vEyePos.xyz);
	float3 vReflection = reflect(vToPoint,float3(0,1,0));

	uint nNum = 40;
	float fStep = 40.0/nNum;

	//
	float4 color = float4(0,0,0,0);
	float value = 1.0f;
	for (uint i=1;i<nNum;++i)
	{
		float3 vPointW = p+vReflection*i*fStep;

		float4 vPointP = mul(float4(vPointW,1.0f),mViewProj);
		vPointP /= vPointP.w;

		float2 uv = vPointP.xy;
		uv += 1.0f;
		uv /= 2.0f;	
		uv.y = 1.0f - uv.y;

		float4 tex = g_texture2.Sample(g_sampler,uv);
		if (tex.a!=0.0f)
		{
			float depthR = mul(float4(vPointW,1.0f),mView).z;
			if (depthR<=-60.0f)
				continue;

			float3 positionT = g_texture0.Sample(g_sampler,uv).xyz;
			float depthT = mul(float4(positionT,1.0f),mView).z;

			if (depthT>depthR)
			{
				color = g_texture5.Sample(g_sampler,uv);
				value = (float)i/(float)nNum;
				break;
			}
		}
		
	}

	float4 environment = g_EnvironmentLight.SampleLevel(g_sampler,vReflection,0);
	color = lerp(color,environment,value);

	result.color = float4(color.xyz,1.0f);

	//
	float4 position = mul(float4(input.positionW.xyz, 1.0f), mViewProj);
	result.depth = position.z/position.w;

	return result;
}
