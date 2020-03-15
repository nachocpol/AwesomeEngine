/*
	Surface.hlsl
	Shaders used to render the meshes.
*/
#include "Declarations.h"
#include "Utils.hlsl"

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

float D_DistributionGGX(float3 NdotH, float roughness)
{
	float r2 = roughness * roughness;
	float NdotH2 = NdotH * NdotH;
	float denom = (NdotH2 * (r2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return r2 / denom;
}

float4 PSSurface(SurfaceVSOut input) : SV_Target0
{
	float3 n = normalize(input.WorldNormal);
	float3 v = normalize(CameraWorldPos - input.WorldPos);
	float3 F0 = float3(0.04, 0.04, 0.04); // TO-DO metallic surface

	float3 baseColor = 1.0;
	float3 total = 0.0;

	for(int lightIdx = 0; lightIdx < NumLights; ++lightIdx)
	{
		Light light = Lights[lightIdx];
		
		float3 diffuse = 0.0;
		float3 specular = 0.0;

		if(light.Type == 0)
		{
			float3 l = normalize(light.PosDirection - input.WorldPos);
			float3 h = normalize(v + l);

			float NdotH = max(dot(n,h),0.0);
			
			// Specular BRDF (Cook-Torrance)
			float D = D_DistributionGGX(NdotH, 0.2);
			//float G;
			//float3 F;

			diffuse	= D;

			//float dist = distance(light.PosDirection, input.WorldPos);
			//float attenuation = 1.0 - saturate(dist / light.Radius);
			//diffuse = light.Color * light.Intensity * attenuation * NdotL;
		}

		total += diffuse + specular;
	}
	

	return float4(total, 1.0);
}