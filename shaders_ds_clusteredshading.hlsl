// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#ifndef COMPUTE_SHADER_TILE_HLSL
#define COMPUTE_SHADER_TILE_HLSL

#define COMPUTE_SHADER_TILE_GROUP_DIM 32
#define COMPUTE_SHADER_TILE_GROUP_SIZE COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM

cbuffer FrameBuffer : register(b0)
{
	uint2 uDispatch;					// x and y dimensions of the Dispatch call
	uint2 uScreen;						// Size of the input Texture2D
	float fValue;
};

struct PointLight
{
    float3 position;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};
cbuffer LightBuffer : register(b1)
{
        float4x4   mViewR;
	float4x4   mProj;
        float4x4   mViewProj;
	PointLight sLight[16];
	uint uLightNum;
}

Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
Texture2D g_texture2 : register(t2);

RWTexture2D<float4> gFramebuffer : register(u0);

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[32];
groupshared uint sTileNumLights;

//
struct SurfaceData
{
    float3 position;             // position
    //float3 positionDX;         // derivatives
    //float3 positionDY;         // of view space position
    float3 normal;               // normal
    float4 albedo;
    float specularAmount;        // Treated as a multiplier on albedo
    float specularPower;
};

float3 DecodeSphereMap(float2 e)
{
    float2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0f * f - 1.0f);
    
    float3 n;
    n.xy = m * (e * 4.0f - 2.0f);
    n.z  = 3.0f - 8.0f * f;
    return n;
}

float3 ComputePositionViewFromZ(float2 positionScreen,
                                float viewSpaceZ)
{
    float2 screenSpaceRay = float2(positionScreen.x / mViewProj._11,
                                   positionScreen.y / mViewProj._22);
    
    float3 positionView;
    positionView.z = viewSpaceZ;
    // Solve the two projection equations
    positionView.xy = screenSpaceRay.xy * positionView.z;
    
    return positionView;
}

SurfaceData ComputeSurfaceDataFromGBufferSample(uint3 positionViewport)
{
    // Load the raw data from the GBuffer
    //GBuffer rawData;
/*
    float4 normal_specular = g_texture0.Load(positionViewport).xyzw;
    float4 albedo = g_texture1.Load(positionViewport).xyzw;
    float2 positionZGrad = g_texture2.Load(positionViewport).xy;
    float zBuffer = g_texture2.Load(positionViewport).z;
    
    float2 gbufferDim = float2(1280,720);
    //uint dummy;
    //gGBufferTextures[0].GetDimensions(gbufferDim.x, gbufferDim.y, dummy);
    
    // Compute screen/clip-space position and neighbour positions
    // NOTE: Mind DX11 viewport transform and pixel center!
    // NOTE: This offset can actually be precomputed on the CPU but it's actually slower to read it from
    // a constant buffer than to just recompute it.
    float2 screenPixelOffset = float2(2.0f, -2.0f) / gbufferDim;
    float2 positionScreen = (float2(positionViewport.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
    float2 positionScreenX = positionScreen + float2(screenPixelOffset.x, 0.0f);
    float2 positionScreenY = positionScreen + float2(0.0f, screenPixelOffset.y);
*/        

    // Decode into reasonable outputs
/*
    SurfaceData data;
        
    // Unproject depth buffer Z value into view space
    float viewSpaceZ = mViewProj._43 / (zBuffer - mViewProj._33);

    data.positionView = ComputePositionViewFromZ(positionScreen, viewSpaceZ);
    data.positionViewDX = ComputePositionViewFromZ(positionScreenX, viewSpaceZ + positionZGrad.x) - data.positionView;
    data.positionViewDY = ComputePositionViewFromZ(positionScreenY, viewSpaceZ + positionZGrad.y) - data.positionView;

    data.normal = DecodeSphereMap(normal_specular.xy);
    data.albedo = albedo;

    data.specularAmount = normal_specular.z;
    data.specularPower = normal_specular.w;
*/
	
    SurfaceData data;
    float3 position = g_texture0.Load(positionViewport).xyz;
    data.albedo = g_texture1.Load(positionViewport);
    float3 normal = g_texture2.Load(positionViewport).xyz;
    data.specularAmount = 0.9f;
    data.specularPower = 25.0f;

    data.position = mul(float4(position, 1.0f), mViewR).xyz;
    data.normal = mul(float4(normal, 1.0f), (float3x3)mViewR).xyz;
    
    return data;
}

float linstep(float minv, float maxv, float v)
{
    return 1.0f - saturate((v - minv) / (maxv - minv));
}

//
// As below, we separate this for diffuse/specular parts for convenience in deferred lighting
void AccumulatePhongBRDF(float3 normal,
                         float3 lightDir,
                         float3 viewDir,
                         float3 lightContrib,
                         float specularPower,
                         inout float3 litDiffuse,
                         inout float3 litSpecular)
{
    // Simple Phong
    float NdotL = dot(normal, lightDir);
    [flatten] if (NdotL > 0.0f) {
        float3 r = reflect(lightDir, normal);
        float RdotV = max(0.0f, dot(r, viewDir));
        float specular = pow(RdotV, specularPower);

        litDiffuse += lightContrib * NdotL;
        litSpecular += lightContrib * specular;
    }
}

// Accumulates separate "diffuse" and "specular" components of lighting from a given
// This is not possible for all BRDFs but it works for our simple Phong example here
// and this separation is convenient for deferred lighting.
// Uses an in-out for accumulation to avoid returning and accumulating 0
void AccumulateBRDFDiffuseSpecular(SurfaceData surface, PointLight light,
                                   inout float3 litDiffuse,
                                   inout float3 litSpecular)
{
/*
    float3 directionToLight = light.positionView - surface.positionView;
    float distanceToLight = length(directionToLight);

    [branch] if (distanceToLight < light.attenuationEnd) {
        float attenuation = linstep(light.attenuationEnd, light.attenuationBegin, distanceToLight);
        directionToLight *= rcp(distanceToLight);       // A full normalize/RSQRT might be as fast here anyways...
        
        AccumulatePhongBRDF(surface.normal, directionToLight, normalize(surface.positionView),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
    }
*/
}

// Uses an in-out for accumulation to avoid returning and accumulating 0
void AccumulateBRDF(SurfaceData surface, PointLight light,
                    inout float3 lit)
{
    // All in view space
    float3 lightView  = mul(float4(light.position.xyz, 1.0f), mViewR).xyz;

    float3 directionToLight = lightView - surface.position;
    float distanceToLight = length(directionToLight);
    
    //
    [branch] if (distanceToLight < light.attenuationEnd) {
        float attenuation = linstep(light.attenuationBegin, light.attenuationEnd, distanceToLight);
        directionToLight *= rcp(distanceToLight);       // A full normalize/RSQRT might be as fast here anyways...
	
	float3 litDiffuse = float3(0,0,0);
        float3 litSpecular = float3(0,0,0);
        AccumulatePhongBRDF(surface.normal, directionToLight, normalize(surface.position),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
        
        lit += surface.albedo.rgb * (litDiffuse);// + surface.specularAmount * litSpecular);
    }
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void CSMain(uint3 groupId          : SV_GroupID,
            uint3 dispatchThreadId : SV_DispatchThreadID,
            uint3 groupThreadId    : SV_GroupThreadID)
{
    //
    SurfaceData surface = ComputeSurfaceDataFromGBufferSample(dispatchThreadId.xyz);

    // NOTE: This is currently necessary rather than just using SV_GroupIndex to work
    // around a compiler bug on Fermi.
    //uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;
    
    // How many total lights?
    //uint totalLights, dummy;
    //gLight.GetDimensions(totalLights, dummy);

    //uint2 globalCoords = dispatchThreadId.xy;

    //SurfaceData surfaceSamples[MSAA_SAMPLES];
    //ComputeSurfaceDataFromGBufferAllSamples(globalCoords, surfaceSamples);
        
    // Work out Z bounds for our samples
    float minZSample = 1000.0f;
    float maxZSample =    1.0f;
    {
        bool validPixel = surface.position.z >= 1.0f && surface.position.z <  1000.0f;
        [flatten] if (validPixel) 
	{
                minZSample = min(minZSample, surface.position.z);
                maxZSample = max(maxZSample, surface.position.z);
	}
    }
    
    // Initialize shared memory light list and Z bounds
    uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;
    if (groupIndex == 0)
    {
        sTileNumLights = 0;
        //sNumPerSamplePixels = 0;
        sMinZ = 0x7F7FFFFF;      // Max float
        sMaxZ = 0;
    }

    GroupMemoryBarrierWithGroupSync();
    
    // NOTE: Can do a parallel reduction here but now that we have MSAA and store sample frequency pixels
    // in shaded memory the increased shared memory pressure actually *reduces* the overall speed of the kernel.
    // Since even in the best case the speed benefit of the parallel reduction is modest on current architectures
    // with typical tile sizes, we have reverted to simple atomics for now.
    // Only scatter pixels with actual valid samples in them
    if (maxZSample >= minZSample) {
        InterlockedMin(sMinZ, asuint(minZSample));
        InterlockedMax(sMaxZ, asuint(maxZSample));
    }

    GroupMemoryBarrierWithGroupSync();

    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);
    
    // NOTE: This is all uniform per-tile (i.e. no need to do it per-thread) but fairly inexpensive
    // We could just precompute the frusta planes for each tile and dump them into a constant buffer...
    // They don't change unless the projection matrix changes since we're doing it in view space.
    // Then we only need to compute the near/far ones here tightened to our actual geometry.
    // The overhead of group synchronization/LDS or global memory lookup is probably as much as this
    // little bit of math anyways, but worth testing.

    // Work out scale/bias from [0, 1]
    float2 tileScale = float2(1280,720) * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
    float2 tileBias = tileScale - float2(groupId.xy);

    // Now work out composite projection matrix
    // Relevant matrix columns for this tile frusta
    float4 c1 = float4(mProj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -mProj._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Derive frustum planes
    float4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  maxTileZ);
    
    // Normalize frustum planes (near/far already normalized)
    [unroll] for (uint i = 0; i < 4; ++i) {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
        
    // Cull lights for this tile
    for (uint lightIndex = groupIndex; lightIndex < uLightNum; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE) {
        PointLight light = sLight[lightIndex];
                
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll] for (uint i = 0; i < 6; ++i) {
            float d = dot(frustumPlanes[i], float4(light.position, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }

        [branch] if (inFrustum) {
            // Append light to list
            // Compaction might be better if we expect a lot of lights
            uint listIndex;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    
    uint numLights = sTileNumLights;
/*
    // Only process onscreen pixels (tiles can span screen edges)
    if (all(globalCoords < mFramebufferDimensions.xy)) {
        [branch] if (mUI.visualizeLightCount) {
            [unroll] for (uint sample = 0; sample < MSAA_SAMPLES; ++sample) {
                float4 color = (float(sTileNumLights) / 255.0f).xxxx;
                if (sTileNumLights >= MAX_SMEM_LIGHTS) color.gb = 0; // red for overflow
                WriteSample(globalCoords, sample, color);
            }
        } else if (numLights > 0) {
            bool perSampleShading = RequiresPerSampleShading(surfaceSamples);
            [branch] if (mUI.visualizePerSampleShading && perSampleShading) {
                [unroll] for (uint sample = 0; sample < MSAA_SAMPLES; ++sample) {
                    WriteSample(globalCoords, sample, float4(1, 0, 0, 1));
                }
            } else {
                float3 lit = float3(0.0f, 0.0f, 0.0f);
                for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex) {
                    PointLight light = gLight[sTileLightIndices[tileLightIndex]];
                    AccumulateBRDF(surfaceSamples[0], light, lit);
                }

                // Write sample 0 result
                WriteSample(globalCoords, 0, float4(lit, 1.0f));
                        
                [branch] if (perSampleShading) {
                    #if DEFER_PER_SAMPLE
                        // Create a list of pixels that need per-sample shading
                        uint listIndex;
                        InterlockedAdd(sNumPerSamplePixels, 1, listIndex);
                        sPerSamplePixels[listIndex] = PackCoords(globalCoords);
                    #else
                        // Shade the other samples for this pixel
                        for (uint sample = 1; sample < MSAA_SAMPLES; ++sample) {
                            float3 litSample = float3(0.0f, 0.0f, 0.0f);
                            for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex) {
                                PointLight light = gLight[sTileLightIndices[tileLightIndex]];
                                AccumulateBRDF(surfaceSamples[sample], light, litSample);
                            }                        
                            WriteSample(globalCoords, sample, float4(litSample, 1.0f));
                        }
                    #endif
                } else {
                    // Otherwise per-pixel shading, so splat the result to all samples
                    [unroll] for (uint sample = 1; sample < MSAA_SAMPLES; ++sample) {
                        WriteSample(globalCoords, sample, float4(lit, 1.0f));
                    }
                }
            }
        } else {
            // Otherwise no lights affect here so clear all samples
            [unroll] for (uint sample = 0; sample < MSAA_SAMPLES; ++sample) {
                WriteSample(globalCoords, sample, float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
        }
    }

    #if DEFER_PER_SAMPLE && MSAA_SAMPLES > 1
        // NOTE: We were careful to write only sample 0 above if we are going to do sample
        // frequency shading below, so we don't need a device memory barrier here.
        GroupMemoryBarrierWithGroupSync();

        // Now handle any pixels that require per-sample shading
        // NOTE: Each pixel requires MSAA_SAMPLES - 1 additional shading passes
        const uint shadingPassesPerPixel = MSAA_SAMPLES - 1;
        uint globalSamples = sNumPerSamplePixels * shadingPassesPerPixel;

        for (uint globalSample = groupIndex; globalSample < globalSamples; globalSample += COMPUTE_SHADER_TILE_GROUP_SIZE) {
            uint listIndex = globalSample / shadingPassesPerPixel;
            uint sampleIndex = globalSample % shadingPassesPerPixel + 1;        // sample 0 has been handled earlier

            uint2 sampleCoords = UnpackCoords(sPerSamplePixels[listIndex]);
            SurfaceData surface = ComputeSurfaceDataFromGBufferSample(sampleCoords, sampleIndex);

            float3 lit = float3(0.0f, 0.0f, 0.0f);
            for (uint tileLightIndex = 0; tileLightIndex < numLights; ++tileLightIndex) {
                PointLight light = gLight[sTileLightIndices[tileLightIndex]];
                AccumulateBRDF(surface, light, lit);
            }
            WriteSample(sampleCoords, sampleIndex, float4(lit, 1.0f));
        }
    #endif
*/

    //
    GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        sTileNumLights = uLightNum;
	for (uint lightindex = 0; lightindex < uLightNum; ++lightindex) 
	{
		sTileLightIndices[lightindex] = lightindex;
	}
    } 

    //
    GroupMemoryBarrierWithGroupSync();
    
    //
    if (dispatchThreadId.x>=uScreen.x)
    {
	if (dispatchThreadId.y>=uScreen.y)
	{    
		return;
	}
    }

    float4 color = g_texture0.Load(dispatchThreadId.xyz);
    if (color.a!=0.0f)
    {
	float3 result = float3(0,0,0);
	for (uint lightindex = 0; lightindex < uLightNum; ++lightindex) 
	{
 		uint lindex = sTileLightIndices[lightindex];
		AccumulateBRDF(surface, sLight[lightindex], result);
	}
	gFramebuffer[dispatchThreadId.xy] = float4(result.xyz,1.0f);
    }

    //
/*
    float3 Value = float3(surface.position.x-surface.normal.x,surface.position.y-surface.normal.y,surface.position.z-surface.normal.z);
    if (Value.x<0.0f)
    {
	Value.x *= -1.0f;
    }    
    if (Value.y<0.0f)
    {
	Value.y *= -1.0f;
    }
    if (Value.z<0.0f)
    {
	Value.z *= -1.0f;
    }
    gFramebuffer[dispatchThreadId.xy] = float4(Value,1.0f);
*/
}

#endif // COMPUTE_SHADER_TILE_HLSL
