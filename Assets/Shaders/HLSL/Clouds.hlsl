#include "Utils.hlsl"

cbuffer AtmosphereData : register(b0)
{
    float4 ViewPosition; 
    float4x4 InvViewProj;
    float4 SunDirection; 
    float CloudBase;
    float CloudExtents;
    float Absorption;
    float CoverageScale;
    float BaseNoiseScale;
    float DetailNoiseScale;
}

Texture2D CloudCoverageTex : register(t0);
Texture3D BaseNoise : register(t1);
Texture3D DetailNoise : register(t2);
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

float FMB5(float3 p, float lacunariy, float gain)
{
    const int o = 5;
    float a = 1.0f;
    float f = 1.0f;
    float t = 0.0f;
    float sum = 0.0f;
    for(int i = 0; i < o; i++)
    {
        sum += ValueNoiseSlow3D(p * f) * a;
        t += a;
        f *= lacunariy;
        a *= gain;
    }
    return sum / t;
}

float GetCloudDensity(float3 p)
{
    float sampleY = saturate((p.y - CloudBase) / CloudExtents); // 0-1
    float fbot = lerp(0.0f,1.0f,clamp(sampleY,0.0f,0.45f) / 0.45f);
    
    // Coverage
    float coverage = CloudCoverageTex.Sample(LinearWrapSampler,float2(p.x,p.z) * CoverageScale);

    // Base noise
    float baseNoise = BaseNoise.Sample(LinearWrapSampler, p * BaseNoiseScale);
    float baseShape = saturate(baseNoise - pow(coverage,1.5f)) * fbot; 
    
    // Detail noise
    float detailNoise = DetailNoise.Sample(LinearWrapSampler, p * DetailNoiseScale);

    return saturate(baseShape - detailNoise * 0.2f);
}

float GetCloudDensityCheap(float3 p)
{
    float sampleY = saturate((p.y - CloudBase) / CloudExtents); // 0-1
    float fbot = lerp(0.0f,1.0f,clamp(sampleY,0.0f,0.45f) / 0.45f);
    
    // Coverage
    float coverage = CloudCoverageTex.Sample(LinearWrapSampler,float2(p.x,p.z) * CoverageScale);

    // Base noise
    float baseNoise = BaseNoise.Sample(LinearWrapSampler, p * BaseNoiseScale);
    float baseShape = saturate(baseNoise - pow(coverage,1.5f)) * fbot; 

    return baseShape;
}

float4 PSClouds(VSOut i): SV_Target0
{
	float M_PI = 3.141516f;
    float MaxFadeDist = 5000.0f;

    float3 toSun = normalize(-SunDirection);
    float3 ro = ViewPosition;
    float3 rd = normalize(mul(InvViewProj,float4(i.TexCoord.xy,1.0f,1.0f)));

    float3 cloudColor = float3(0.0f,0.0f,0.0f);
    float transmitance = 1.0f;

    float cloudStartDist = 0.0f;
    bool intersection = RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase,0.0f),ro,rd,cloudStartDist);
    if(intersection)
    {
        float mu = dot(rd, normalize(SunDirection));
        float g = -0.76f; 
        float phaseM = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(1.0f + g * g - 2.0f * g * mu, 1.5f)); 

        float3 cloudEntry = ro + rd * cloudStartDist;
        float cloudEndDist = 0.0f;
        RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase + CloudExtents,0.0f),cloudEntry,rd,cloudEndDist);
        float3 cloudExit = cloudEntry + rd * cloudEndDist;
        float travelDist = distance(cloudEntry,cloudExit);
        
        const int steps = 64;
        float stepSize = travelDist / float(steps);
        float lightStepSize = 8.0f;
        float3 p = cloudEntry;
        
        // Precalculate the dir * lenght:
        float3 rdScaled = rd * stepSize;
        float3 toSunScaled = toSun * lightStepSize;
        // Precalculate Absorption * lenght:
        float absorptionScaled = Absorption * stepSize;
        float absorptionScaledLight = Absorption * lightStepSize;
        // Powder precal:
        float powderScaled = Absorption * stepSize * 2.0f;

        // Cloud marching:
        for(int i=0; i<steps; i++)
        {
            float density = GetCloudDensity(p);
            float ti = exp(-absorptionScaled  * density);
            transmitance *= ti;

            // Light sampling:
            float3 lightColor = float3(0.0f,0.0f,0.0f);
            float lightTransmitance = 1.0f;
            float3 lightPos = p + toSunScaled;
            [loop]
            for(int j=0;j<8;j++)
            {
                float lightDensity = GetCloudDensityCheap(lightPos);
                float lightCurTransmitance = exp(-absorptionScaledLight * lightDensity);
                lightTransmitance *= lightCurTransmitance;
                lightPos = lightPos + toSun * lightStepSize;
                if(lightPos.y > CloudBase + CloudExtents || lightTransmitance < 0.001f)
                {
                    break;
                }
            }

            float powder = 1.0f - (-powderScaled * density);
            float3 stepLight = float3(100.0f,100.0f,100.0f) * lightTransmitance * phaseM * powder;
            float3 stepColor = stepLight * density * stepSize;
            cloudColor += stepColor * transmitance;

            if(transmitance < 0.0001f )
            {
                break;
            }
            p = p + rdScaled;
        }
    }

    float opacity = 1.0f - transmitance;
    opacity *= 1.0f - saturate(cloudStartDist / MaxFadeDist);
    return float4(cloudColor,opacity);
}