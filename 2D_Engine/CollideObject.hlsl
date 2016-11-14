RWTexture2D<float4> output : register(u0);

cbuffer CONST_BUFFER : register(b0)
{

	float2 corner;
	float width;
	float height;

}

RWStructuredBuffer<int> count : register(u1);
RWStructuredBuffer<int> object : register(u2);

[numthreads(32, 32, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{

	int tmp;

	if (threadID.x < corner.x + width && threadID.x >= corner.x)
		if (threadID.y < corner.y + height && threadID.y >= corner.y) {

			if (output[threadID.xy].w != 0) {
			
				InterlockedAdd(count[0], 1, tmp);
				InterlockedMin(object[0], output[threadID.xy].w * 255, tmp);

			}
		
		}


}