/*
	Surface.hlsl
	Shaders used to render the meshes.
*/
#include "Declarations.h"
#include "BRDF.hlsl"

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

SamplerState LinearWrapSampler : register(s0);
SamplerState LinearClampSampler : register(s1);

TextureCube<float4> IrradianceMap  : register(t1);
TextureCube<float4> PrefilteredMap : register(t2);
Texture2D<float4> BRDFLut		   : register(t3);

//////////////////////////////////////
// Surface
//////////////////////////////////////

SurfaceVSOut VSSurface(SurfaceVSIn input)
{
	SurfaceVSOut output;
	output.WorldPos = mul(gItemData.World, float4(input.Position, 1.0)).xyz;
	output.ClipPos = mul(gCameraData.InvViewProj, float4(output.WorldPos, 1.0));
	output.WorldNormal = mul(gItemData.World, float4(input.Normal, 0.0));
	output.LocalPos = input.Position;
	return output;
}

float4 PSSurface(SurfaceVSOut input) : SV_Target0
{
	float3 n = normalize(input.WorldNormal);
	float3 v = normalize(gCameraData.CameraWorldPos - input.WorldPos);
	float3 r = reflect(-v, n);
	float NdotV = max(dot(n, v), 0.0);

	float3 F0 = float3(0.04, 0.04, 0.04); 
	F0 = lerp(F0, gItemData.BaseColor, gItemData.Metalness);

	float3 diffuseColor = gItemData.BaseColor * (1.0 - gItemData.Metalness);
	float roughness = max(gItemData.Roughness * gItemData.Roughness, 0.001); // Roughness remaping.

	float3 total = 0.0;

	for(int lightIdx = 0; lightIdx < gItemData.NumLights; ++lightIdx)
	{
		Light light = Lights[lightIdx];
		
		if(light.Type == 0)
		{
			// Light
			float lightNormDist = 1.0 - (distance(input.WorldPos, light.PosDirection) / light.Radius);
			float attenuation = lightNormDist * lightNormDist; // TODO: revisit this
			float3 lightRadiance = light.Color * attenuation * light.Intensity * 5.0;

			// BRDF
			float3 l = normalize(light.PosDirection - input.WorldPos);
			float3 h = normalize(v + l);
			float NdotH = max(dot(n, h), 0.0);
			float NdotL = max(dot(n, l), 0.0);
			float LdotH = max(dot(l, h), 0.0);
			float HdotV = max(dot(h, v), 0.0);
			
			// Specular BRDF (Cook-Torrance)
			// TODO: multi scatering spec (loosing energy rough surfaces!)
			float D = D_DistributionGGX(NdotH, roughness);
			float G  = G_GeometrySmith(NdotV, NdotL, roughness);
			float3 F = F_FresnelSchlick(HdotV, F0);
			float3 BRDFs = D * G * F / max(4.0 * NdotV * NdotL, 0.0001);

			// Diffuse BRDF (Lambert diffuse)
			#if 0
			float3 BRDFd = diffuseColor * Fd_Lambert();
			#else
			float3 BRDFd = diffuseColor * Fd_Burley(NdotV, NdotL, LdotH, roughness);
			#endif

			// Ratio of refraction:
			float3 kS = F;
			float3 kD = (float3(1.0,1.0,1.0) - kS) * (1.0 - gItemData.Metalness);

			total += (kD * BRDFd + BRDFs) * lightRadiance * NdotL;
		}
	}

	// Ambient term:
	float3 ambient = 0;
	{
		float3 kS = F_FresnelSchlickRoughness(NdotV, F0, roughness);
		float3 kD = 1.0 - kS;

		float3 irradiance = IrradianceMap.SampleLevel(LinearWrapSampler, n, 0).rgb;
		irradiance *= diffuseColor;

		float2 brdfLUT = BRDFLut.SampleLevel(LinearClampSampler, float2(NdotV, roughness), 0).xy;
		float3 prefiltered = PrefilteredMap.SampleLevel(LinearWrapSampler, r, roughness * 4).rgb;
		float3 specular = prefiltered * (kS * brdfLUT.x + brdfLUT.y);

		ambient = kD * irradiance + specular;
	}
	total += ambient;

	return float4(total, 1.0);
}