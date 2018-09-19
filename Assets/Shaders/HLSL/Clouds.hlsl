#include "Utils.hlsl"

cbuffer AtmosphereData : register(b0)
{
    float4 ViewPosition; 
    float4x4 InvViewProj; 
}

Texture3D BaseNoiseTex : register(t0);
SamplerState LinearWrapSampler : register(s0);

struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

float CloudPlane(float3 p, float h)
{
    return abs(dot(p,float3(0.0f,-1.0f,0.0f))) + h;
}

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
	
    float3 ro = ViewPosition;
    float3 rd = normalize(mul(InvViewProj,float4(i.TexCoord.xy,1.0f,1.0f)));

    float3 p = ro;
    float cd = 1.0f;
    float3 cloudColor = float3(0.0f,0.0f,0.0f);
    float opacity = 0.0f;
    for(int i=0;i<64;i++)
    {
        p = ro + rd * cd;
        float cloud = CloudPlane(p,150.0f);
        if(cloud < 0.1f)
        {
            opacity = BaseNoiseTex.Sample(LinearWrapSampler,float3(p.xz * 0.0005f,8.0f));
            cloudColor = float3(1.0f,1.0f,1.0f);
            float fade = saturate(distance(p,ViewPosition) / 1000.0f);
            opacity *= 1.0f - fade;
            break;
        }
        cd += cloud;
    }

    return float4(cloudColor,opacity);
}