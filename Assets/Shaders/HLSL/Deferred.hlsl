cbuffer AppData : register(b0)
{
	float4x4 Model;
	float4x4 View;
	float4x4 Projection;
	float4 DebugColor;
}

cbuffer MaterialInfo: register (b1)
{
	float4 AlbedoColor;
	int UseBumpTex;
}

cbuffer	LightInfo : register (b1)
{
	float4 LightPosition; // xyz(For directional, this is direction) w(type)
	float4 LightColor;    // w not used
	float LightRange;	  // Only for spot and point
	float LightAngle;	  // Only for Spot
}

SamplerState LinearWrapSampler : register(s0);

Texture2D AlbedoTexture  : register(t0);
Texture2D BumpTexture    : register(t1);

Texture2D GColor    : register	(t0);
Texture2D GNormals 	: register	(t1);
Texture2D GPosition : register	(t2);



struct VSInGBuffer
{
	float3 Position : POSITION;
	float3 Normal 	: NORMAL;
	float3 Tangent 	: TANGENT;
	float2 Texcoord : TEXCOORD;
};

struct VSOutGBuffer
{
	float4 ClipPos  : SV_Position;
	float4 WPos		: WPOS;
	float3 PNormal  : NORMAL;
	float2 PTexcoord: TEXCOORD;
	float3x3 TBN	: TBNMATRIX;
};

struct PSOutGBuffer
{
	float4 Color    : SV_Target0;
	float4 Normals  : SV_Target1;
	float4 Position : SV_Target2;
};

VSOutGBuffer VSGBuffer(VSInGBuffer i)
{
	VSOutGBuffer o;
	o.WPos = mul(Model,float4(i.Position,1.0f));
	o.ClipPos = mul(Projection,mul(View,o.WPos));
	o.PNormal = normalize(i.Normal);
	o.PTexcoord = i.Texcoord;

	if(UseBumpTex)
	{
		float3 T 	= normalize(mul(Model,float4(i.Tangent,0.0f))).xyz;
		float3 N 	= normalize(mul(Model,float4(i.Normal,0.0f))).xyz;
		float3 B 	= cross(T,N);
		o.TBN 		= float3x3(T,B,N);
	}
	
	return o;
}

PSOutGBuffer PSGBuffer(VSOutGBuffer i)
{	
	float4 c = AlbedoTexture.Sample(LinearWrapSampler,i.PTexcoord);
	float3 n;
	if(UseBumpTex)
	{
		n = BumpTexture.Sample(LinearWrapSampler,i.PTexcoord).xyz;
		n = normalize(n * 2.0f - 1.0f);
		n = normalize(mul(i.TBN, n));
	}
	else
	{
		n = i.PNormal.xyz;
	}

	PSOutGBuffer o;
	o.Color = c * AlbedoColor;
	o.Normals = float4(n,1.0f);
	o.Position = i.WPos;

	return o;
}

struct VSInLightPass
{
	float4 Position : POSITION;
};

struct VSOutLightPass
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

VSOutLightPass VSLightPass(VSInLightPass i)
{
	VSOutLightPass o;
	o.ClipPos = i.Position;
	o.TexCoord = 0.5f * (i.Position.xy + 1.0f);
	o.TexCoord.y = 1.0 - o.TexCoord.y;
	return o;
}

float4 PSLightPass(VSOutLightPass i): SV_Target0
{
	float4 pcol = GColor.Sample(LinearWrapSampler, i.TexCoord);
	float3 pnorm = GNormals.Sample(LinearWrapSampler, i.TexCoord).xyz;

	float ndl = 0.0f;

	if(LightPosition.w == 0.0f) // Directional
	{
		ndl = max(dot(pnorm.xyz,-LightPosition.xyz),0.0f);
	}
	else if(LightPosition.w == 1.0f) // Point
	{
		
	}
	else if(LightPosition.w == 2.0f) // Spot
	{
		
	}
	else
	{

	}

	float4 ambient = pcol * 0.15f;
	float4 finalColor = pcol* LightColor * ndl;
	finalColor += ambient;
	return finalColor;
}