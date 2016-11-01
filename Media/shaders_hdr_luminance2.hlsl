//-----------------------------------------------------------------------------
// File: csReduceToFloat.hlsl
//
// Desc: Reduces input buffer1D to a single float by a factor of group threads.
//
//-----------------------------------------------------------------------------

groupshared float			g_total[128];

RWStructuredBuffer<float>		g_src 		: register(u0);
RWStructuredBuffer<float>		g_result 	: register(u1);

[numthreads(128, 1, 1)]
void CSMain( uint3 groupId : SV_GroupID, uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex )
{
	g_total[groupIndex] = g_src[dispatchId.x];

	//
	// Parallel reduction algorithm
	//

	GroupMemoryBarrierWithGroupSync();
	uint k = 64;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 32;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 16;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 8;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 4;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 2;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	GroupMemoryBarrierWithGroupSync();
	k = 1;
	if (groupIndex < k)
	{
		g_total[groupIndex] += g_total[k + groupIndex];
	}

	if (groupIndex == 0)
	{
		g_result[groupId.x] = g_total[0];
	}
}
