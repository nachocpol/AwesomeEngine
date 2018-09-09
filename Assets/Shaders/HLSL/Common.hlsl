cbuffer UIData : register(b0)
{
	float4x4 Projection;	// ImGUI ortho projection
}

Texture2D MainTex : register(t0);
SamplerState LinearWrapSampler : register(s0);


// Generic blit to screen
struct VSIn
{
	float4 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

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

// User interface ImGUI
struct VSInUI
{
	float2 Position 	: POSITION;
	float2 TexCoord 	: TEXCOORD;
	float4 VertexColor 	: COLOR;
};

struct VSOutUI
{
	float4 ClipPos 		: SV_Position;
	float4 VertexColor 	: COLOR;
	float2 TexCoord    	: TEXCOORD;
};

VSOutUI VSUI(VSInUI i)
{
	VSOutUI o;
	o.ClipPos = mul(Projection,float4(i.Position,0.0f,1.0f));
	o.VertexColor = i.VertexColor;
	o.TexCoord = i.TexCoord;
	return o;
}

float4 PSUI(VSOutUI i): SV_Target0
{
	return MainTex.Sample(LinearWrapSampler,i.TexCoord) * i.VertexColor;
}