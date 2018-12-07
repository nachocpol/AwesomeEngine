#include "Utils.hlsl"

cbuffer CloudsData : register(b0)
{
    float4 ViewPosition; 
    float4x4 InvViewProj;
    float4 SunDirection; 
    float CloudBase;
    float CloudExtents;
    float Absorption;
    float CloudsScale;
    float EarthR;
    float Time;
}

Texture2D CloudCoverageTex : register(t0);
Texture3D BaseNoise : register(t1);
Texture3D DetailNoise : register(t2);
Texture3D CloudShadow : register(t3);
SamplerState LinearWrapSampler : register(s0);

RWTexture2D<float4> CloudOutput : register(u0);
RWTexture3D<float> CloudShadowOutput : register(u1);

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
    float sampleY = saturate((p.y - (CloudBase )) / (CloudExtents)); // 0-1
    float fbot = lerp(0.0f,1.0f,clamp(sampleY,0.0f,0.45f) / 0.45f);

    // Coverage
    float coverage = CloudCoverageTex.SampleLevel(LinearWrapSampler,float2(p.x,p.z) * CloudsScale, 0).x;

    // Base noise
    float baseNoise = BaseNoise.SampleLevel(LinearWrapSampler, p * CloudsScale * 2.0f, 0).x;
    float baseShape = saturate(baseNoise - pow(abs(coverage),1.5f)) * fbot; 

    // Detail noise
    // Comes packed as R11G11B10
    p.z -= Time * 0.01f;
    float3 detailNoisePk = DetailNoise.SampleLevel(LinearWrapSampler, p * CloudsScale * 20.0f, 0).xyz;
    float detailNoise = (detailNoisePk.x * 0.65f) + (detailNoisePk.y * 0.25f) + (detailNoisePk.z * 0.15f);

    return saturate(baseShape - detailNoise * 0.25f);
}

float4 RenderClouds(float2 texCoords)
{
    float M_PI = 3.141516f;
    float MaxFadeDist = 9000.0f;

    float3 toSun = normalize(-SunDirection).xyz;
    float3 ro = ViewPosition.xyz;
    float3 rd = normalize(mul(InvViewProj,float4(texCoords,1.0f,1.0f))).xyz;

    float3 cloudColor = float3(0.0f,0.0f,0.0f);
    float transmitance = 1.0f;
    float cloudStartDist = 0.0f;
    bool intersection = RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase,0.0f),ro,rd,cloudStartDist);
    if(intersection && cloudStartDist < MaxFadeDist)
    {
        float mu = dot(rd, normalize(SunDirection).xyz);
        float g = -0.76f; 
        float phaseM = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(abs(1.0f + g * g - 2.0f * g * mu), 1.5f)); 

        float3 cloudEntry = ro + rd * cloudStartDist;
        float cloudEndDist = 0.0f;
        RayPlane(float3(0.0f,1.0f,0.0f),float3(0.0f,CloudBase + CloudExtents,0.0f),cloudEntry,rd,cloudEndDist);
        float3 cloudExit = cloudEntry + rd * cloudEndDist;
        float travelDist = distance(cloudEntry,cloudExit);
        
        const int steps = 64;
        float stepSize = travelDist / float(steps);
        float3 p = cloudEntry;
        
        // Precal some values:
        float3 rdScaled = rd * stepSize;
        float absorptionScaled = Absorption * stepSize;
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
            float3 shadowTc = p * CloudsScale;
            shadowTc.y = (p.y - CloudBase) / CloudExtents;
            lightTransmitance = CloudShadow.SampleLevel(LinearWrapSampler,shadowTc.xzy,0).x;
            
            float powder = 1.0f - (-powderScaled * density);
            float3 stepLight = float3(100.0f,60.0f,40.0f) * lightTransmitance * phaseM * powder;
            float3 stepColor = stepLight * density * stepSize;
            cloudColor += stepColor * transmitance;

            p = p + rdScaled;

            if(transmitance < 0.001f || p.y >= CloudBase + CloudExtents)
            {
                break;
            }
        }
    }

    float opacity = 1.0f - transmitance;
    opacity *= 1.0f - saturate(cloudStartDist / MaxFadeDist);
    return float4(cloudColor,opacity);
}

[numthreads(32,32,1)]
void CSClouds(uint3 threadID : SV_DispatchThreadID)
{
    uint2 dim = uint2(0,0);
    CloudOutput.GetDimensions(dim.x,dim.y);
    float2 texCoord =  float2(threadID.xy) / float2(dim);    
    if(threadID.x >= dim.x || threadID.y >= dim.y)
    {
        return;
    }
    texCoord.y = 1.0f - texCoord.y;
    CloudOutput[threadID.xy] = RenderClouds(texCoord);
}

[numthreads(16,16,4)]
void CSCloudShadow(uint3 groudID: SV_GroupID,uint3 threadID : SV_GroupThreadID)
{
    uint3 pos = (groudID * uint3(16,16,4)) + threadID;
    uint3 dim = uint3(0,0,0);
    CloudShadowOutput.GetDimensions(dim.x,dim.y,dim.z);
    float3 texCoord =  float3(pos) / float3(dim);    

    // Current world position:
    float3 wpos = texCoord / CloudsScale;
    wpos.z =  lerp(CloudBase,CloudBase + CloudExtents, texCoord.z);

    float tmpz = wpos.z;
    wpos.z = wpos.y;
    wpos.y = tmpz;

    // Light sampling:
    float3 toSun = normalize(-SunDirection).xyz;
    float lightStepSize = 8.0f;
    float absorptionScaledLight = Absorption * lightStepSize;
    float lightTransmitance = 1.0f;
    float3 toSunScaled = toSun * lightStepSize;
    float3 lightPos = wpos + toSunScaled;
    [loop]
    for(int j=0;j<16;j++)
    {
        float lightDensity = GetCloudDensity(lightPos);
        float lightCurTransmitance = exp(-absorptionScaledLight * lightDensity);
        lightTransmitance *= lightCurTransmitance;
        lightPos = lightPos + toSun * lightStepSize;
        if(lightPos.y > CloudBase + CloudExtents || lightTransmitance < 0.0001f)
        {
            break;
        }
    }

    CloudShadowOutput[pos] = lightTransmitance;
}

float4 PSClouds(VSOut i): SV_Target0
{
    return RenderClouds(i.TexCoord);
}