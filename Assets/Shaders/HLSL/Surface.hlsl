/*
	Surface.hlsl
	Shaders used to render the meshes.
*/
#include "Declarations.h"

struct SurfaceVSIn
{
	float3 Position : POSITION;
	float3 Normal 	: NORMAL;
	float3 Tangent 	: TANGENT;
	float2 Texcoord : TEXCOORD;
};

struct SurfaceVSOut
{
	float4 ClipPos  	: SV_Position;
	float3 WorldPos		: WPOS;
	float3 WorldNormal  : NORMAL;
	//float2 PTexcoord: TEXCOORD;
	//float3x3 TBN	: TBNMATRIX;
};

//////////////////////////////////////
// Surface
//////////////////////////////////////

SurfaceVSOut VSSurface(SurfaceVSIn input)
{
	SurfaceVSOut output;
	output.WorldPos = mul(World, float4(input.Position, 1.0)).xyz;
	output.ClipPos = mul(InvViewProj, float4(output.WorldPos, 1.0));
	output.WorldNormal = mul(World, float4(input.Normal, 0.0));
	return output;
}

float4 PSSurface(SurfaceVSOut input) : SV_Target0
{
	float3 baseColor = 1.0;
	float3 N = normalize(input.WorldNormal);
	float3 total = 0.0;

	for(int lightIdx = 0; lightIdx < NumLights; ++lightIdx)
	{
		Light light = Lights[lightIdx];
		
		float3 diffuse = 0.0;
		float3 specular = 0.0;

		if(light.Type == 0)
		{
			float3 L = normalize(light.PosDirection - input.WorldPos);
			float NdotL = max(dot(N,L), 0.0);
	
			float dist = distance(light.PosDirection, input.WorldPos);
			float attenuation = 1.0 - saturate(dist / light.Radius);
			diffuse = light.Color * light.Intensity * attenuation * NdotL;
		}

		total += diffuse + specular;
	}
	

	return float4(total, 1.0);
}