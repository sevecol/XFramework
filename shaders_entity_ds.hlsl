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
	float4x4 mWorldView;
	float4x4 mWorldViewProj;
	float4x4 mWorldViewProjInv;
};

PSInput VSMain(VSInput input)
{
	PSInput result;

	float3 position = float3(input.position.x,input.position.y,input.position.z);
	float3 normal = float3(input.normal.x,input.normal.y,input.normal.z);
	
	result.position = mul(float4(position, 1.0f), mWorldViewProj);
	result.positionW 	= position.xyz;
	result.normal  		= normal.xyz;
	result.tangent		= input.tangent;

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

// Data that we can read or derive from the surface shader outputs
struct SurfaceData
{
    float3 position;             // position
    float3 positionDX;           // derivatives
    float3 positionDY;           // of view space position
    float3 normal;               // normal
    float4 albedo;
    float specularAmount;        // Treated as a multiplier on albedo
    float specularPower;
};

SurfaceData ComputeSurfaceDataFromGeometry(PSInput input)
{
    SurfaceData surface;
/*
    surface.positionView = input.positionV;

    // These arguably aren't really useful in this path since they are really only used to
    // derive shading frequencies and composite derivatives but might as well compute them
    // in case they get used for anything in the future.
    // (Like the rest of these values, they will get removed by dead code elimination if
    // they are unused.)
    surface.positionViewDX = ddx_coarse(surface.positionView);
    surface.positionViewDY = ddy_coarse(surface.positionView);

    // Optionally use face normal instead of shading normal
    //float3 faceNormal = ComputeFaceNormal(input.positionW);
    surface.normal = input.normalV;
    
    surface.albedo = g_txDiffuse.Sample(g_sampler, input.uv);
    surface.albedo.a = 1.0f;
    //surface.albedo.rgb = mUI.lightingOnly ? float3(1.0f, 1.0f, 1.0f) : surface.albedo.rgb;

    // Map NULL diffuse textures to white
    //uint2 textureDim;
    //gDiffuseTexture.GetDimensions(textureDim.x, textureDim.y);
    //surface.albedo = (textureDim.x == 0U ? float4(1.0f, 1.0f, 1.0f, 1.0f) : surface.albedo);

    // We don't really have art asset-related values for these, so set them to something
    // reasonable for now... the important thing is that they are stored in the G-buffer for
    // representative performance measurement.
    surface.specularAmount = 0.9f;
    surface.specularPower = 25.0f;
*/
    return surface;
}

float2 EncodeSphereMap(float3 n)
{
    float oneMinusZ = 1.0f - n.z;
    float p = sqrt(n.x * n.x + n.y * n.y + oneMinusZ * oneMinusZ);
    return n.xy / p * 0.5f + 0.5f;
}

PsOutput PSMain(PSInput input)
{
	PsOutput result;
/*
	SurfaceData surface = ComputeSurfaceDataFromGeometry(input);

    	result.color0 = float4(EncodeSphereMap(surface.normal),
                                           surface.specularAmount,
                                           surface.specularPower);
    	result.color1 = surface.albedo;
	result.color1.a = 1.0f;
    	result.color2.xy = float2(ddx_coarse(surface.positionView.z),
                                         ddy_coarse(surface.positionView.z));
	result.color2.z = input.position.z;
*/
	//
	float3 binormal = cross(input.normal.xyz,input.tangent.xyz);
	float3x3 tbn;
	tbn[0] = input.tangent;
	tbn[1] = binormal;
	tbn[2] = input.normal;

	//
	float3 texnormal = g_txNormal.Sample(g_sampler, input.uv).xyz;
	texnormal *= 2.0f;
	texnormal -= 1.0f;

	// 
	float3 normal = mul(texnormal,tbn);
	normal = lerp(normal,input.normal,1.0f);

	result.color0 = float4(input.positionW.xyz,1.0f);
	result.color1 = g_txDiffuse.Sample(g_sampler, input.uv);
	result.color2 = float4(normalize(normal),1.0f);

	return result;
}
