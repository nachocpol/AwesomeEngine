/*
	DebugDraw.hlsl
*/

#include "Declarations.h"


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