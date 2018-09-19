#include "Utils.hlsl"

struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

VSOut VSClouds(VSIn i)
{
	VSOut o;
	o.ClipPos = float4(i.Position.xy,1.0f,1.0f);
	o.TexCoord = i.Position.xy;
	return o;
}

float4 PSClouds(VSOut i): SV_Target0
{
	float M_PI = 3.141516f;
	
    //float3 ro = View;
    //ro.y += EarthR;
    //float3 rd = normalize(mul(InvViewProj,float4(i.TexCoord.xy,1.0f,1.0f)));

    return float4(1.0f,0.0f,0.0f,1.0f);
}