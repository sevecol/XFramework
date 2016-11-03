
#define BLOCK_SIZE_X	8
#define BLOCK_SIZE_Y	8
#define BLOCK_THREADS	(BLOCK_SIZE_X * BLOCK_SIZE_Y)

groupshared float			g_Total[BLOCK_THREADS];

cbuffer FrameBuffer : register(b0)
{
	uint2 uDispatch;					// x and y dimensions of the Dispatch call
	uint2 uScreen;						// Size of the input Texture2D
	float fValue;
};

Texture2D				g_Texture : register(t0);
SamplerState g_sampler : register(s0);
RWStructuredBuffer<float>		g_Result  : register(u0);

static const float4 			c_luminance			= float4(.299, .587, .114, 0);

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void CSMain( uint3 groupId : SV_GroupID, uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex )
{
	// Read texel data without any filtering or sampling.
	uint3  location1	= uint3(dispatchId.xy, 0);
	uint3  location2	= uint3(dispatchId.xy + uint2(BLOCK_SIZE_X * uDispatch.x, 0), 0);
	uint3  location3	= uint3(dispatchId.xy + uint2(0, BLOCK_SIZE_Y * uDispatch.y), 0);
	uint3  location4	= uint3(dispatchId.xy + uint2(BLOCK_SIZE_X * uDispatch.x, BLOCK_SIZE_Y * uDispatch.y), 0);

	float4 color1 = g_Texture.Load(location1);
	float4 color2 = g_Texture.Load(location2);
	float4 color3 = g_Texture.Load(location3);
	float4 color4 = g_Texture.Load(location4);

	//
	float4 sum = color1 + color2 + color3 + color4;
	g_Total[groupIndex] = dot(sum, c_luminance);

	// Reduction
	GroupMemoryBarrierWithGroupSync();
	uint k = 32;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 16;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 8;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 4;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 2;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 1;
	if (groupIndex < k)
	{
		g_Total[groupIndex] += g_Total[k + groupIndex];
	}

	if (groupIndex == 0)
	{
		g_Result[groupId.y * uDispatch.x + groupId.x] = g_Total[0];
	}
}