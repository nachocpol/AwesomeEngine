/*
	IBL.hlsl
*/
#include "Utils.hlsl"

SamplerState LinearWrapSampler : register(s0);

Texture2D SourceEnvMap : register(t0);
RWTexture2DArray<float4> TargetMip : register(u0);

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Low discrepancy sequence generation:
float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness; // Rougness remaping.

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float3 GetSamplingNormal(uint3 threadID)
{
	uint2 threadPos = threadID.xy;
	uint face = threadID.z;
	uint2 dim = uint2(0, 0);
	uint layers = 0;
	TargetMip.GetDimensions(dim.x, dim.y, layers);
	float2 texCoord = (float2)threadPos / (float2)dim;

	// Sampling direction (as if this was +X face)
	float2 xface[2] = { float2(1,1), float2(-1,-1) };
	float3 samplingNormal = float3(
		1.0,
		lerp(xface[0].y, xface[1].y, texCoord.y),
		lerp(xface[0].x, xface[1].x, texCoord.x)
	);

	// Rotate to align with each cube face:
	float3x3 faceRot[6];
	faceRot[0] = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	faceRot[1] = float3x3(cos(PI), 0, sin(PI), 0, 1, 0, -sin(PI), 0, cos(PI));
	faceRot[2] = float3x3(cos(-PI / 2), -sin(-PI / 2), 0, sin(-PI / 2), cos(-PI / 2), 0, 0, 0, 1);
	faceRot[3] = float3x3(cos(PI / 2), -sin(PI / 2), 0, sin(PI / 2), cos(PI / 2), 0, 0, 0, 1);
	faceRot[4] = float3x3(cos(PI / 2), 0, sin(PI / 2), 0, 1, 0, -sin(PI / 2), 0, cos(PI / 2));
	faceRot[5] = float3x3(cos(-PI / 2), 0, sin(-PI / 2), 0, 1, 0, -sin(-PI / 2), 0, cos(-PI / 2));

	// Fix Y +/- faces
	faceRot[2] = mul(faceRot[2], faceRot[4]);
	faceRot[3] = mul(faceRot[3], faceRot[4]);

	// Rotate base normal(align with current cube face):
	return mul(normalize(samplingNormal), faceRot[face]);
}

[numthreads(8,8,1)]
void CSIrradianceGen(uint3 threadID : SV_DispatchThreadID)
{
	float3 samplingNormal = GetSamplingNormal(threadID);
	float3 irradiance = 0;
	int numSamples = 0;
	float sampleDelta = 0.05;
	
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, samplingNormal);
	up = cross(samplingNormal, right);
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// Spherical -> cartesian:
			float3 cartesian = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			// Rotate it along the direction:
			float3 dir = cartesian.x * right + cartesian.y  * up + cartesian.z * samplingNormal;

			float2 tc = ToEquirectangular(dir);
			// Using HDR shows extremly ugly pattern where it samples the sun
			// TBF, we probably don't want to sample the sun within the cubemap, ?
			irradiance += ReinhardTonemap(SourceEnvMap.SampleLevel(LinearWrapSampler, tc, 0).rgb) * cos(theta) * sin(theta);
			//irradiance += SourceEnvMap.SampleLevel(LinearWrapSampler, tc, 0).rgb * cos(theta) * sin(theta);
			numSamples++;
		}
	}
	irradiance = irradiance * PI / ((float)numSamples);

	// To equirectangular and sample:
	// float2 tc = ToEquirectangular(samplingDir);
	// float3 color = SourceEnvMap.SampleLevel(LinearWrapSampler, tc, 0).xyz;

	TargetMip[threadID] = float4(irradiance,1);
}

[numthreads(8,8,1)]
void CSPrefilterGen(uint3 threadID : SV_DispatchThreadID)
{

}