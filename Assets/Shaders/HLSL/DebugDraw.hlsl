/*
	DebugDraw.hlsl
*/

#include "Declarations.h"

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
};

//////////////////////////////////////
// Shaders to render debug meshes
//////////////////////////////////////
DebugDrawVSOut VSDebugDraw(DebugDrawVSIn i)
{
	DebugDrawVSOut o;
	o.ClipPos = mul(InvViewProj, mul(World, float4(i.Position, 1.0)));
	o.Color.rgb = i.Color;
	o.Color.a = 1.0;
	return o;
}

float4 PSDebugDraw(DebugDrawVSOut i) : SV_Target0
{
	return i.Color * DebugColor;
}