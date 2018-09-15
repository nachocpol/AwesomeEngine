cbuffer UIData : register(b0)
{
	float4x4 Projection;	// ImGUI ortho projection
}

Texture2D MainTex : register(t0);
SamplerState LinearWrapSampler : register(s0);


// Generic blit to screen
struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

VSOut VSFullScreen(VSIn i)
{
	VSOut o;
	o.ClipPos = float4(i.Position.xy,1.0f,1.0f);
	o.TexCoord = 0.5f * (i.Position.xy + 1.0f);
	o.TexCoord.y = 1.0 - o.TexCoord.y;
	return o;
}

float4 PSFullScreen(VSOut i): SV_Target0
{
	return MainTex.Sample(LinearWrapSampler, i.TexCoord);
}

// Generic blit with tonemap and gamma correction
float4 ToSRGB(float4 base, float gamma)
{
	float grcp = 1.0f/ gamma;
	return pow(base,float4(grcp,grcp,grcp,grcp));
}

float3 Uncharted2Tonemap(float3 x)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	float W = 11.2f;
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float4 PSToneGamma(VSOut i): SV_Target0
{
	float4 base = MainTex.Sample(LinearWrapSampler, i.TexCoord);
	base *= 8.0f;
	base.xyz = Uncharted2Tonemap(base.xyz);
	base.w = 1.0f;
	base = ToSRGB(base,2.2f);
	return base;
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