#include "Utils.hlsl"

cbuffer AtmosphereData : register(b0)
{
	float4 View;			// xyz: camera view, z: not used
	float4x4 ViewMatrix; 
	float4x4 InvViewProj; 
	float4 ViewPosition;
	float3 SunDirection;
	float EarthR;
	float AtmosR;
    float Hr;
    float Hm;
    float SunIntensity;
    float3 BetaR;
    float Pad0;
    float3 BetaM;
    float Pad1;
}

struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos  : SV_Position;
	float2 TexCoord : TEXCOORD;
};

VSOut VSAtmosphere(VSIn i)
{
	VSOut o;
	o.ClipPos = float4(i.Position.xy,1.0f,1.0f);
	o.TexCoord = i.Position.xy;
	return o;
}

float4 PSAtmosphere(VSOut i): SV_Target0
{
	float M_PI = 3.141516f;
	
    float3 ro = View;
    ro.y += EarthR;
    float3 rd = normalize(mul(InvViewProj,float4(i.TexCoord.xy,1.0f,1.0f)));
	
	float atmosLenght = RaySphere(float3(0.0f,0.0f,0.0f), AtmosR, ro, rd).x;
	float3 atmosColor = float3(0.0f,0.0f,0.0f);
	if(atmosLenght != -1.0f)
	{
		int numSamples = 16;
		int numLightSamples = 8;

		float stepSize = atmosLenght / float(numSamples);
		float current = 0.0f;
		float3 sumR = float3(0.0f,0.0f,0.0f);
		float3 sumM = float3(0.0f,0.0f,0.0f);
    	float opticalDepthR = 0.0f;
    	float opticalDepthM = 0.0f; 
    	
    	float mu = dot(rd, normalize(SunDirection)); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction 
    	float phaseR = 3.0f / (16.0f * M_PI) * (1.0f + mu * mu); 
    	float g = 0.76f; 
    	float phaseM = 3.0f / (8.0f * M_PI) * ((1.0f - g * g) * (1.0f + mu * mu)) / ((2.0f + g * g) * pow(1.0f + g * g - 2.0f * g * mu, 1.5f)); 

    	for(int i=0; i<numSamples; i++)
    	{
    		float3 curP = ro + (current + stepSize * 0.5f) * rd;
    		float height = curP.y - EarthR;

    		// compute optical depth for light
        	float hr = exp(-height / Hr) * stepSize; 
        	float hm = exp(-height / Hm) * stepSize; 
        	opticalDepthR += hr; 
        	opticalDepthM += hm; 

        	// light optical depth
        	float distLight = RaySphere(float3(0.0f,0.0f,0.0f), AtmosR, curP, normalize(SunDirection)).x; 
        	float segmentLengthLight = distLight / float(numLightSamples);
        	float tCurrentLight = 0.0f; 
        	float opticalDepthLightR = 0.0f;
        	float opticalDepthLightM = 0.0f; 
        	int j = 0;
        	for (j = 0; j < numLightSamples; j++) 
        	{ 
        	    float3 samplePositionLight = curP + (tCurrentLight + segmentLengthLight * 0.5f) * normalize(SunDirection); 
        	    // we had a .lenght here also above... hum
        	    float heightLight = samplePositionLight.y - EarthR; 
        	    if (heightLight < 0.0f)
        	    {
        	    	break; 	
        	    } 
        	    opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight; 
        	    opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight; 
        	    tCurrentLight += segmentLengthLight; 
        	} 
        	if(j==numLightSamples)
        	{
        		float3 tau = BetaR * (opticalDepthR + opticalDepthLightR) + BetaM * 1.1f * (opticalDepthM + opticalDepthLightM); 
            	float3 attenuation = float3(exp(-tau.x), exp(-tau.y), exp(-tau.z)); 
            	sumR += attenuation * hr; 
            	sumM += attenuation * hm; 
        	}

            current += stepSize; 
    	}

		atmosColor.xyz = (sumR * BetaR * phaseR + sumM * BetaM * phaseM) * SunIntensity;
	}

	return float4(atmosColor,1.0f);
}