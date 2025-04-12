#include "..\ShaderDeclarationMacros.h"

CBUFFER(CubeConstants)
	float4x4 Model;
	float4x4 ViewProjection;
CBUFFER_END(CubeConstants, 0)

#include "..\ShaderDeclarationMacrosEnd.h"

#if !defined(__cplusplus)

struct VSInput
{
	float3 Position : POSITION;
	//float3 Color    : COLOR;
};

struct VSOutSimple
{
	float4 Position : SV_Position;
	float3 Color    : COLOR;
};

VSOutSimple VSCube(VSInput i)
{
	VSOutSimple o;
	o.Position = mul(gCubeConstants.ViewProjection, mul(gCubeConstants.Model, float4(i.Position, 1.0)));
	o.Color = i.Position * 0.5 + 0.5;
	return o;
}

float4 PSCube(VSOutSimple i) : SV_Target0
{
	return float4(i.Color,1.0f);
}

#endif
