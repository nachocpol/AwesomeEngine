/*
	Simple.hlsl
*/

struct VSInput
{
	float3 Position : POSITION;
	float3 Color    : COLOR;
};

struct VSOutSimple
{
	float4 Position : SV_Position;
	float3 Color    : COLOR;
};

VSOutSimple VSSimple(VSInput i)
{
	VSOutSimple o;
	o.Position  = float4(i.Position,1.0);
	o.Color 	= i.Color;
	return o;
}

float4 PSSimple(VSOutSimple i) : SV_Target0
{
	return float4(i.Color,1.0f);
}