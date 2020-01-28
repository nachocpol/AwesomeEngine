/*
	DebugDraw.hlsl
*/

#include "Declarations.hlsl"

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
	float4 ClipPos 		: SV_Position;
	float4 Color 	: COLOR;
};

//////////////////////////////////////
// Shaders to render debug meshes
//////////////////////////////////////
DebugDrawVSOut VSDebugDraw(DebugDrawVSIn i)
{
	DebugDrawVSOut o;
	o.ClipPos = mul(InvViewProj, float4(i.Position, 1.0)); // Inputs come in world space
	o.Color.rgb = i.Color;
	o.Color.a = 1.0;
	return o;
}

float4 PSDebugDraw(DebugDrawVSOut i) : SV_Target0
{
	return i.Color;
}