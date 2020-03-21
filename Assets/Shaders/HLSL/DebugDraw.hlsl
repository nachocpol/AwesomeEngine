/*
	DebugDraw.hlsl
*/

#include "Surface.hlsl"

SamplerState LinearWrapSampler : register(s0);

TextureCube<float4> CubeMapSource : register(t0);

//////////////////////////////////////
// Vertex inputs and outputs definitions
//////////////////////////////////////
struct DebugDrawVSIn
{
	float3 Position : POSITION;
	float3 Color 	: COLOR;
};

struct DebugDrawVSOut
{
	float4 ClipPos 	: SV_Position;
	float4 Color 	: COLOR;
	float3 LocalPos : LPOS;
};

//////////////////////////////////////
// Shaders to render debug meshes (with lines)
//////////////////////////////////////
DebugDrawVSOut VSDebugDraw(DebugDrawVSIn input)
{
	DebugDrawVSOut output;
	output.ClipPos = mul(InvViewProj, mul(World, float4(input.Position, 1.0)));
	output.Color.rgb = input.Color;
	output.Color.a = 1.0;
	output.LocalPos = input.Position;
	return output;
}

float4 PSDebugDraw(DebugDrawVSOut input) : SV_Target0
{
	return input.Color * DebugColor;
}

//////////////////////////////////////
// Shaders to render debug meshes solid
//////////////////////////////////////

float4 PSDebugDrawSolid(SurfaceVSOut input) : SV_Target0
{
	if (DebugCubemap == 1)
	{
		float3 dir = normalize(input.LocalPos);
		float3 cubemapColor = CubeMapSource.SampleLevel(LinearWrapSampler, dir, 0).xyz;
		return float4(cubemapColor, 1.0);
	}
	else
	{
		return DebugColor;
	}
}