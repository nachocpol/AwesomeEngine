cbuffer AtmosphereData : register(b0)
{
	float4 View;	// xyz: camera view, z: not used
}

struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

VSOut VSAtmosphere(VSIn i)
{
	VSOut o;
	o.ClipPos = float4(i.Position.xy,1.0f,1.0f);
	o.TexCoord = i.Position;
	return o;
}

float4 PSAtmosphere(VSOut i): SV_Target0
{
	float3 rd = normalize(View.xyz * float3(i.TexCoord.xy,1.5f));

	float horizon = max(dot(rd,float3(0.0f,0.0f,1.0f)),0.0f);
	return float4(View.xyz ,1.0f);
}