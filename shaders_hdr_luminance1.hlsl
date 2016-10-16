

RWStructuredBuffer<float>		g_Result0 : register(u0);


[numthreads(1, 1, 1)]
void CSMain( uint3 groupId : SV_GroupID, uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex )
{
	if (groupIndex == 0)
	{
		g_Result0[0] = 12.0f;
		//g_Result1[0] = 4.0f;
	}

}