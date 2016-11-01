
#define BLOCK_SIZE_X	32
#define BLOCK_SIZE_Y	32
#define BLOCK_THREADS	(BLOCK_SIZE_X * BLOCK_SIZE_Y)

cbuffer FrameBuffer1 : register(b0)
{
	uint2 uDispatch;					// x and y dimensions of the Dispatch call
	uint2 uScreen;						// Size of the input Texture2D
	float fValue;
};

RWTexture2D<float4> gFramebuffer : register(u0);

struct SPixelLink
{
	float4 color;
	float depth;
	uint next;
};
RWStructuredBuffer<SPixelLink> gPixelLinkBuffer : register(u1);
RWStructuredBuffer<uint> gStartOffsetBuffer : register(u2);

#define MAX_PIXELS 	4

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void CSMain( uint3 groupId : SV_GroupID, uint3 dispatchId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex )
{
        if (dispatchId.x>=1280)
        {

	    	return;
        }	
	if (dispatchId.y>=720)
	{    
		return;
	}

	float2 position = dispatchId.xy;
	if (dispatchId.x>=640)
	{
		position.x -= 640; 
	}
	else
	{
		position.x += 640; 
	}

	uint uOffset = dispatchId.y*1280 + position.x;
	uint uCounter = gStartOffsetBuffer[uOffset];

	SPixelLink SortedPixels[MAX_PIXELS];

	uint uNumPixels = 0;
	while (uCounter!=0xFFFFFFFF)
	{
		SortedPixels[uNumPixels  ].color = gPixelLinkBuffer[uCounter].color;
		SortedPixels[uNumPixels++].depth = gPixelLinkBuffer[uCounter].depth;

		uCounter = (uNumPixels>=MAX_PIXELS)?0xFFFFFFFF:gPixelLinkBuffer[uCounter].next;
	}

	if (uNumPixels==0)
	{
		return;
	}

	// SortPixel
	if (uNumPixels>=2)
	{
		for (uint i=0;i<uNumPixels-1;i++)
		{
			for (uint j=0;j<uNumPixels-i-1;j++)
			{
				if (SortedPixels[j].depth>SortedPixels[j+1].depth)
				{
					SPixelLink temp;
					temp.color = SortedPixels[j].color;
					temp.depth = SortedPixels[j].depth;

					SortedPixels[j].color = SortedPixels[j+1].color;
					SortedPixels[j].depth = SortedPixels[j+1].depth;

					SortedPixels[j+1].color = temp.color;
					SortedPixels[j+1].depth = temp.depth;
				}
			}
		}
	}

	float4 fFinalColor = gFramebuffer[dispatchId.xy];
	for (int i=0;i<uNumPixels;i++)
	{
		fFinalColor.xyz = SortedPixels[i].color.xyz * SortedPixels[i].color.w + fFinalColor.xyz * (1-SortedPixels[i].color.w);
	}

	gFramebuffer[dispatchId.xy] = fFinalColor;
}