/*
	IBL.hlsl
*/
#include "Utils.hlsl"

SamplerState LinearWrapSampler : register(s0);

Texture2D SourceEnvMap : register(t0);
RWTexture2DArray<float4> TargetMip : register(u0);

[numthreads(8,8,1)]
void CSIrradianceGen(uint3 threadID : SV_DispatchThreadID)
{
	uint2 threadPos = threadID.xy;
	uint2 dim = uint2(0,0);
	uint layers = 0;
    TargetMip.GetDimensions(dim.x,dim.y,layers);
	float2 texCoord = (float2)threadPos / (float2)dim;

	float3 color = 0.0;

	// Positive x:
	{
		float2 xface[2] = { float2(1,1), float2(-1,-1) };
		float3 curVec = float3(
			1.0,
			lerp(xface[0].x, xface[1].x, texCoord.x),
			lerp(xface[0].y, xface[1].y, texCoord.y)
		);
		float2 tc = ToEquirectangular(normalize(curVec));
		color = SourceEnvMap.SampleLevel(LinearWrapSampler, tc, 0).xyz;
	}

	TargetMip[threadID] = float4(color,1);
}

[numthreads(1,1,1)]
void CSConvolute(uint3 threadID : SV_DispatchThreadID)
{

}