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

float GeometrySchlickGGX(float NdotV, float k)
{
	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float G_GeometrySmith(float NdotV, float NdotL, float k)
{
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 F_FresnelSchlick(float NdotV, float3 F0)
{
	float f = pow(1.0 - NdotV, 5.0);
    return f + F0 * (1.0 - f);
}

float4 PSSurface(SurfaceVSOut input) : SV_Target0
{
	float3 n = normalize(input.WorldNormal);
	float3 v = normalize(CameraWorldPos - input.WorldPos);
	float3 F0 = float3(0.04, 0.04, 0.04); // TO-DO metallic surface

	float r = max(Roughness * Roughness, 0.001); // Roughness remaping.

	float3 total = 0.0;

	for(int lightIdx = 0; lightIdx < NumLights; ++lightIdx)
	{
		Light light = Lights[lightIdx];
		
		if(light.Type == 0)
		{
			// Light
			float lightNormDist = 1.0 - (distance(input.WorldPos, light.PosDirection) / light.Radius);
			float attenuation = lightNormDist * lightNormDist;
			float3 lightRadiance = light.Color * attenuation * light.Intensity;

			// BRDF
			float3 l = normalize(light.PosDirection - input.WorldPos);
			float3 h = normalize(v + l);
			float NdotH = max(dot(n,h), 0.0);
			float NdotV = max(dot(n,v), 0.0);
			float NdotL = max(dot(n,l), 0.0);
			float VdotH = max(dot(v,h), 0.0);
			
			// Specular BRDF (Cook-Torrance)
			float D = D_DistributionGGX(NdotH, r);
			float G  = G_GeometrySmith(NdotV, NdotL, r);
			float3 F = F_FresnelSchlick(NdotV, F0);
			float3 BRDFs = D * G * F / max(4.0 * NdotV * NdotL, 0.0001);

			// Diffuse BRDF (Lambert diffuse)
			float3 BRDFd = BaseColor / PI;

			// Ratio of refraction:
			float3 kS = F;
			float3 kD = (1.0 - kS) * (1.0 - Metalness);

			total += (kD * BRDFd + BRDFs) * lightRadiance * NdotL;
			
			float3 ambient = BRDFd * kD * 0.01;
			total += ambient;
		}
	}
	return float4(total, 1.0);
}