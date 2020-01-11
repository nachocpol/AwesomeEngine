cbuffer AppData : register(b0)
{
	float4x4 Model;
	float4x4 View;
	float4x4 Projection;
	float4 DebugColor;
}

Texture2D AlbedoTexture : register(t0);
Texture2D BumpTexture : register(t1);
SamplerState LinearWrapSampler : register(s0);

struct VSIn
{
	float3 Position : POSITION;
	//float3 Normal 	: NORMAL;
	//float3 Tangent 	: TANGENT;
	//float2 Texcoord : TEXCOORD;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float4 WPos		: WPOS;
	//float3 PNormal  : NORMAL;
	//float2 PTexcoord: TEXCOORD;
	//float3x3 TBN	: TBNMATRIX;
};

VSOut VSFordwardSimple(VSIn i)
{
	VSOut o;
	o.WPos = mul(Model,float4(i.Position,1.0f));
	o.ClipPos = mul(Projection,mul(View,o.WPos));
	//o.PNormal = normalize(i.Normal);
	//o.PTexcoord = i.Texcoord;

	//float3 T 	= normalize(mul(Model,float4(i.Tangent,0.0f))).xyz;
	//float3 N 	= normalize(mul(Model,float4(i.Normal,0.0f))).xyz;
	//T 			= normalize(T - dot(T, N) * N);
	//float3 B 	= cross(T,N);
	//o.TBN 		= float3x3(T,B,N);
	
	return o;
}

float4 PSFordwardSimple(VSOut i): SV_Target0
{	
	/*
	float4 c = AlbedoTexture.Sample(LinearWrapSampler,i.PTexcoord);
	float3 n = BumpTexture.Sample(LinearWrapSampler,i.PTexcoord).xyz;;
	n = normalize(n * 2.0f - 1.0f);
	n = normalize(mul(i.TBN, n));

	float3 tl = normalize(float3(0.0f,15.0f,8.0f) - i.WPos.xyz);
	float ndl = max(dot(n,tl),0.0f);

	return (c * ndl);
	*/
	return float4(1,0,1,1);
}