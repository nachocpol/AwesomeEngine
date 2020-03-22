/*
	DebugDraw.hlsl
*/

#include "Surface.hlsl"

TextureCube<float4> CubeMapSource : register(t0);
Texture2D<float4> TextureSource : register(t1);

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
	output.ClipPos = mul(gCameraData.InvViewProj, mul(gItemData.World, float4(input.Position, 1.0)));
	output.Color.rgb = input.Color;
	output.Color.a = 1.0;
	output.LocalPos = input.Position;
	return output;
}

float4 PSDebugDraw(DebugDrawVSOut input) : SV_Target0
{
	return input.Color * gItemData.DebugColor;
}

//////////////////////////////////////
// Shaders to render debug meshes solid
//////////////////////////////////////

float4 PSDebugDrawSolid(SurfaceVSOut input) : SV_Target0
{
	if (gDebugData.Equirectangular == 1)
	{
		float3 dir = normalize(input.LocalPos);
		float2 tc = ToEquirectangular(dir);
		float3 equiColor = TextureSource.SampleLevel(LinearWrapSampler, tc, 0).rgb;
		return float4(equiColor, 1.0);
	}
	else if (gDebugData.DebugCubemap == 1)
	{
		float3 dir = normalize(input.LocalPos);
		float3 cubemapColor = CubeMapSource.SampleLevel(LinearWrapSampler, dir, 0).rgb;
		return float4(cubemapColor, 1.0);
	}
	else
	{
		return gItemData.DebugColor;
	}
}