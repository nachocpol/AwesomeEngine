/*
	BRDF.hlsl
*/

#include "Utils.hlsl"

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
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float G_GeometrySmith(float NdotV, float NdotL, float k)
{
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 F_FresnelSchlick(float cosTheta, float3 F0)
{
	float f = pow(1.0 - cosTheta, 5.0);
	return f + F0 * (1.0 - f);
}

float3 F_FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float F_Schlick(float NdotV, float F0, float F90)
{
	// check this, ndotv, hdok
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