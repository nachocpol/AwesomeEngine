struct VSIn
{
	float4 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

Texture2D MainTex : register(t0);
SamplerState LinearWrapSampler : register(s0);

VSOut VSFullScreen(VSIn i)
{
	VSOut o;
	o.ClipPos = i.Position;
	o.TexCoord = 0.5f * (i.Position.xy + 1.0f);
	o.TexCoord.y = 1.0 - o.TexCoord.y;
	return o;
}

float4 PSFullScreen(VSOut i): SV_Target0
{
	return MainTex.Sample(LinearWrapSampler, i.TexCoord);
}