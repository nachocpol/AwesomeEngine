/*
	AdvancedSample.hlsl
*/

cbuffer DrawData : register(b0)
{
	float Time;
}

cbuffer OtherData : register(b1)
{
	float3 Off;
}

Texture2D MainTex : register(t0);
SamplerState LinearWrapSampler : register(s0);

struct VSInput
{
	float3 Position : POSITION;
	float3 Color    : COLOR;
	float2 TexCoord : UV;
};

struct VSOutSimple
{
	float4 Position : SV_Position;
	float3 Color    : COLOR;
	float2 TexCoord : UV;
};

VSOutSimple VSSimple(VSInput i)
{
	VSOutSimple o;
	o.Position  = float4(i.Position,1.0);
	o.Color 	= i.Color;
	o.TexCoord 	= i.TexCoord;
	return o;
}

float4 PSSimple(VSOutSimple i) : SV_Target0
{
	float4 c = MainTex.Sample(LinearWrapSampler,i.TexCoord);
	//float4 c = float4(i.TexCoord.x,i.TexCoord.y,0.0f,1.0f);
	return float4(c);
}