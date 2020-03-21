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
	float3 LocalPos		: LOCALPOS;
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
	output.LocalPos = input.Position;
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

float F_Schlick(float NdotV, float F0, float F90)
{
	return F0 + (F90 - F0) * pow(1.0 - NdotV, 5.0);
}

float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness) 
{
	float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
	float lightScatter = F_Schlick(NdotL, 1.0, f90);
	float viewScatter = F_Schlick(NdotV, 1.0, f90);
	return lightScatter * viewScatter * (1.0 / PI);
}

float Fd_Lambert()
{
	return 1.0 / PI;
}

float4 PSSurface(SurfaceVSOut input) : SV_Target0
{
	float3 n = normalize(input.WorldNormal);
	float3 v = normalize(CameraWorldPos - input.WorldPos);
	float3 F0 = float3(0.04, 0.04, 0.04); // TO-DO metallic surface

	float3 diffuseColor = BaseColor * (1.0 - Metalness);
	float roughness = max(Roughness * Roughness, 0.001); // Roughness remaping.

	float3 total = 0.0;

	for(int lightIdx = 0; lightIdx < NumLights; ++lightIdx)
	{
		Light light = Lights[lightIdx];
		
		if(light.Type == 0)
		{
			// Light
			float lightNormDist = 1.0 - (distance(input.WorldPos, light.PosDirection) / light.Radius);
			float attenuation = lightNormDist * lightNormDist; // TODO: revisit this
			float3 lightRadiance = light.Color * attenuation * light.Intensity;

			// BRDF
			float3 l = normalize(light.PosDirection - input.WorldPos);
			float3 h = normalize(v + l);
			float NdotH = max(dot(n,h), 0.0);
			float NdotV = max(dot(n,v), 0.0);
			float NdotL = max(dot(n,l), 0.0);
			float VdotH = max(dot(v,h), 0.0);
			float LdotH = max(dot(l,h), 0.0);
			
			// Specular BRDF (Cook-Torrance)
			// TODO: multi scatering spec (loosing energy rough surfaces!)
			float D = D_DistributionGGX(NdotH, roughness);
			float G  = G_GeometrySmith(NdotV, NdotL, roughness);
			float3 F = F_FresnelSchlick(NdotV, F0);
			float3 BRDFs = D * G * F / max(4.0 * NdotV * NdotL, 0.0001);

			// Diffuse BRDF (Lambert diffuse)
			#if 0
			float3 BRDFd = diffuseColor * Fd_Lambert();
			#else
			float3 BRDFd = diffuseColor * Fd_Burley(NdotV, NdotL, LdotH, roughness);
			#endif

			// Ratio of refraction:
			float3 kS = F;
			float3 kD = (float3(1.0,1.0,1.0) - kS) * (1.0 - Metalness);

			total += (kD * BRDFd + BRDFs) * lightRadiance * NdotL;
			
			float3 ambient = BRDFd * kD * 0.01;
			total += ambient;
		}
	}
	return float4(total, 1.0);
}