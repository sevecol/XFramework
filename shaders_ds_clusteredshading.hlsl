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

//#include "GBuffer.hlsl"
//#include "FramebufferFlat.hlsl"
//#include "ShaderDefines.h"

Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
Texture2D g_texture2 : register(t2);
RWTexture2D<float4> gFramebuffer : register(u0);

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[32];
groupshared uint sTileNumLights;

//[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
[numthreads(1, 1, 1)]
void CSMain(uint3 groupId          : SV_GroupID,
                         uint3 dispatchThreadId : SV_DispatchThreadID,
                         uint3 groupThreadId    : SV_GroupThreadID
                         )
{
    float4 color = g_texture0.Load(dispatchThreadId.xyz);
    if (color.a!=0.0f)
    {
	gFramebuffer[dispatchThreadId.xy] = color;
    }

    //gFramebuffer[dispatchThreadId.y*1280+dispatchThreadId.x].fR = 1.0f;
    
    //gFramebuffer[1] = dispatchThreadId.y;

    // NOTE: This is currently necessary rather than just using SV_GroupIndex to work
    // around a compiler bug on Fermi.
    //uint groupIndex = groupThreadId.y * COMPUTE_SHADER_TILE_GROUP_DIM + groupThreadId.x;
    
    // How many total lights?
    //uint totalLights, dummy;
    //gLight.GetDimensions(totalLights, dummy);

    //uint2 globalCoords = dispatchThreadId.xy;
/*
    SurfaceData surfaceSamples[MSAA_SAMPLES];
    ComputeSurfaceDataFromGBufferAllSamples(globalCoords, surfaceSamples);
        
    // Work out Z bounds for our samples
    float minZSample = mCameraNearFar.y;
    float maxZSample = mCameraNearFar.x;
    {
        [unroll] for (uint sample = 0; sample < MSAA_SAMPLES; ++sample) {
            // Avoid shading skybox/background or otherwise invalid pixels
            float viewSpaceZ = surfaceSamples[sample].positionView.z;
            bool validPixel = 
                 viewSpaceZ >= mCameraNearFar.x &&
                 viewSpaceZ <  mCameraNearFar.y;
            [flatten] if (validPixel) {
                minZSample = min(minZSample, viewSpaceZ);
                maxZSample = max(maxZSample, viewSpaceZ);
            }
        }
    }
    
    // Initialize shared memory light list and Z bounds
    if (groupIndex == 0) {
        sTileNumLights = 0;
        sNumPerSamplePixels = 0;
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
    float2 tileScale = float2(mFramebufferDimensions.xy) * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
    float2 tileBias = tileScale - float2(groupId.xy);

    // Now work out composite projection matrix
    // Relevant matrix columns for this tile frusta
    float4 c1 = float4(mCameraProj._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -mCameraProj._22 * tileScale.y, tileBias.y, 0.0f);
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

#if MORE_CULLING
    // heuristic plane separation test
    float minZminX = -(1 - float(groupId.x - 1) / tileScale.x)*minTileZ / mCameraProj._11;
    float minZmaxX = -(1 - float(groupId.x + 1) / tileScale.x)*minTileZ / mCameraProj._11;
    float minZminY = (1 - float(groupId.y - 1) / tileScale.y)*minTileZ / mCameraProj._22;
    float minZmaxY = (1 - float(groupId.y + 1) / tileScale.y)*minTileZ / mCameraProj._22;

    float maxZminX = -(1 - float(groupId.x - 1) / tileScale.x)*maxTileZ / mCameraProj._11;
    float maxZmaxX = -(1 - float(groupId.x + 1) / tileScale.x)*maxTileZ / mCameraProj._11;
    float maxZminY = (1 - float(groupId.y - 1) / tileScale.y)*maxTileZ / mCameraProj._22;
    float maxZmaxY = (1 - float(groupId.y + 1) / tileScale.y)*maxTileZ / mCameraProj._22;
    
    float3 minZcenter = { (minZminX + minZmaxX) / 2, (minZminY + minZmaxY) / 2, minTileZ };
    float3 maxZcenter = { (maxZminX + maxZmaxX) / 2, (maxZminY + maxZmaxY) / 2, maxTileZ };
    float3 center = (minZcenter + maxZcenter) / 2;
#endif
        
    // Cull lights for this tile
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE) {
        PointLight light = gLight[lightIndex];
                
        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll] for (uint i = 0; i < 6; ++i) {
            float d = dot(frustumPlanes[i], float4(light.positionView, 1.0f));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }
        
#if MORE_CULLING
        float3 normal = center - light.positionView;
        normal /= length(normal);
        // compute distance of all corners to the tangent plane, with a few shortcuts (saves 14 muls)
        float min_d1 = -dot(normal, light.positionView);
        float min_d2 = min_d1;
        min_d1 += min(normal.x * minZminX, normal.x * minZmaxX);
        min_d1 += min(normal.y * minZminY, normal.y * minZmaxY);
        min_d1 += normal.z * minTileZ;
        min_d2 += min(normal.x * maxZminX, normal.x * maxZmaxX);
        min_d2 += min(normal.y * maxZminY, normal.y * maxZmaxY);
        min_d2 += normal.z * maxTileZ;
        float min_d = min(min_d1, min_d2);
        bool separated = min_d > light.attenuationEnd;
        if (separated) inFrustum = false;
#endif

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
}

#endif // COMPUTE_SHADER_TILE_HLSL
