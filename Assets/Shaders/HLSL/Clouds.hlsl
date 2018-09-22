#include "Utils.hlsl"

cbuffer AtmosphereData : register(b0)
{
    float4 ViewPosition; 
    float4x4 InvViewProj;
    float4 SunDirection; 
    float CloudBase;
    float CloudExtents;
    float Absorption;
}

Texture2D CloudCoverageTex : register(t0);
Texture3D BaseNoiseTex : register(t1);
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
    return dot(p,float3(0.0f,-1.0f,0.0f)) + h;
}

VSOut VSClouds(VSIn i)
{
	VSOut o;
	o.ClipPos = float4(i.Position.xy,1.0f,1.0f);
	o.TexCoord = i.Position.xy;
	return o;
}

float GetCloudDensity(float3 p)
{
    float sampleY = saturate((p.y - CloudBase) / CloudExtents);
    float scale = 0.0002f;
    float s = CloudCoverageTex.Sample(LinearWrapSampler,float2(p.x,p.z)* scale);
    s = saturate(saturate(s - 0.5) * 10.0);

    float detailScale = scale * 50.0f;    
    float detail = BaseNoiseTex.Sample(LinearWrapSampler,float3(p.x* detailScale,p.z* detailScale,p.y* detailScale)).r;
    //detail = saturate(pow(detail-0.2f,4.0f));

    float hfade = pow(1.0f - sampleY,5.0f);
    
    return s * detail;
}

float4 PSClouds(VSOut i): SV_Target0
{
	float M_PI = 3.141516f;
    
    float3 toSun = normalize(-SunDirection);
    float3 ro = ViewPosition;
    float3 rd = normalize(mul(InvViewProj,float4(i.TexCoord.xy,1.0f,1.0f)));

    float3 cloudColor = float3(0.0f,0.0f,0.0f);
    float opacity = 0.0f;
    float transmitance = 1.0f;

    float cloudStartDist = 0.0f;
    if(RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase,0.0f),ro,rd,cloudStartDist))
    {
        float mu = dot(rd, normalize(SunDirection));
        float g = -0.76f; 
        float phaseM = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(1.0f + g * g - 2.0f * g * mu, 1.5f)); 

        float3 cloudEntry = ro + rd * cloudStartDist;
        float cloudEndDist = 0.0f;
        RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase + CloudExtents,0.0f),cloudEntry,rd,cloudEndDist);
        float3 cloudExit = cloudEntry + rd * cloudEndDist;
        float travelDist = distance(cloudEntry,cloudExit);
        const int steps = 128;
        float stepSize = travelDist / float(steps);
        float3 p = cloudEntry;
        [loop]
        for(int i=0; i<steps; i++)
        {
            float density = GetCloudDensity(p);
            float ti = exp(-Absorption * stepSize * density);
            transmitance *= ti;

            float3 lightColor = float3(0.0f,0.0f,0.0f);
            float lightTransmitance = 1.0f;
            float lightStepSize = 8.0f;
            float3 lightPos = p + toSun * lightStepSize;
            [loop]
            for(int j=0;j<8;j++)
            {
                float lightDensity = GetCloudDensity(lightPos);
                float lightCurTransmitance = exp(-Absorption * lightStepSize * lightDensity);
                lightTransmitance *= lightCurTransmitance;
                lightPos = lightPos + toSun * lightStepSize;
                if(lightPos.y > CloudBase + CloudExtents || lightTransmitance < 0.001f)
                {
                    break;
                }
            }
            float3 stepLight = float3(1.0f,1.0f,1.0f) * lightTransmitance * phaseM;
            float3 stepColor = stepLight * density * stepSize;
            cloudColor += stepColor;

            if(transmitance < 0.0001f)
            {
                break;
            }
            p = p + rd * stepSize;
        }
    }
    opacity = 1.0f - transmitance;
    opacity *= 1.0f - saturate(cloudStartDist / 7000.0f);
    return float4(cloudColor,opacity);
}