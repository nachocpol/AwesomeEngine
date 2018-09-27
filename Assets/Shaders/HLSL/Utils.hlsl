/*
	Utils.hlsl
	Just a bunch of functions
*/

float4 ToSRGB(float4 base, float gamma)
{
	float grcp = 1.0f/ gamma;
	return pow(base,float4(grcp,grcp,grcp,grcp));
}

float4 ToLinearRGB(float4 base,float gamma)
{
	return pow(base,float4(gamma,gamma,gamma,gamma));
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

bool RayPlane(float3 n, float3 p0, float3 l0, float3 l, inout float t) 
{ 
    // assuming vectors are all normalized
    float denom = dot(n, l); 
    if (denom > 1e-6) 
    {
        float3 p0l0 = p0 - l0; 
        t = dot(p0l0, n) / denom; 
        return (t >= 0.0f); 
    } 
    return false; 
} 

/*
	Calculates rd intersection points with the sphere 
	defined by origin (so) and radius (sr)
*/
float2 RaySphere(float3 so,float sr,float3 ro,float3 rd)
{    
	float3 L = so - ro; 
    float tca = dot(L,rd); 
    float d2 = dot(L,L) - tca * tca; 
    float radius2 = sr * sr;
    if (d2 > radius2) 
    {
        return float2(-1.0f,-1.0f); 
    }
    float thc = sqrt(radius2 - d2); 
    
    float2 t;
    t.x = tca - thc; 
    t.y = tca + thc; 
	if (t.x > t.y)
    {
        float tmp = t.x;
        t.x = t.y;
        t.y = tmp;
    }
 
	if (t.x < 0.0f) 
    { 
		t.x = t.y; // If t0 is negative, let's use t1 instead 
		if (t.x < 0.0f) 
        {
            // Both t0 and t1 are negative 
            return float2(-1.0f,-1.0f); 
        }
	} 
	return t; 
}

float3x3 LookAt(float3 origin, float3 dir)
{
 	float3 rr = float3(0.0f,1.0f,0.0f);
 	float3 ww = normalize(dir - origin);
 	float3 uu = normalize(cross(ww, rr));
 	float3 vv = normalize(cross(uu, ww));	
 	return float3x3(uu, vv, ww);
}

float GetProjectionPlane(float fovRad)
{
	float halfFov = fovRad * 0.5f;
	return (1.0f / sin(halfFov)) * cos(halfFov);
}

float Remap(float value,float oldMin,float oldMax,float newMin,float newMax)
{
	return newMin + (value - oldMin) / (oldMax - oldMin) * (newMax - newMin);
}

float Hash1D(float p)
{
	p = 25.0f * frac(p * 0.012f + 0.71f);
	return frac(p * p);
}

float Hash2D(float2 p)
{
	p = 25.0f * frac(p * 0.012f + float2(0.71f,0.12f));
	return frac(p.x * p.y * (p.x + p.y));
}

float Hash3D(float3 p)
{
	p = 25.0f * frac(p * 0.012f + float3(0.71f,0.12f,0.53f));
	return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float ValueNoiseSlow2D(float2 p)
{
	int2 ip = floor(p);
	float2 fp = frac(p);

	float v00 = Hash2D(float2(ip + int2(0,0)));
	float v10 = Hash2D(float2(ip + int2(1,0)));
	float v01 = Hash2D(float2(ip + int2(0,1)));
	float v11 = Hash2D(float2(ip + int2(1,1)));

	return lerp(lerp(v00,v10,fp.x),lerp(v01,v11,fp.x),fp.y);
}

float ValueNoiseSlow3D(float3 p)
{
	int3 ip = floor(p);
	float3 fp = frac(p);

	float v000 = Hash3D(float3(ip + int3(0,0,0)));
	float v100 = Hash3D(float3(ip + int3(1,0,0)));
	float v010 = Hash3D(float3(ip + int3(0,1,0)));
	float v110 = Hash3D(float3(ip + int3(1,1,0)));

	float v001 = Hash3D(float3(ip + int3(0,0,1)));
	float v101 = Hash3D(float3(ip + int3(1,0,1)));
	float v011 = Hash3D(float3(ip + int3(0,1,1)));
	float v111 = Hash3D(float3(ip + int3(1,1,1)));

	float d0 = lerp(lerp(v000,v100,fp.x),lerp(v010,v110,fp.x),fp.y);
	float d1 = lerp(lerp(v001,v101,fp.x),lerp(v011,v111,fp.x),fp.y);
	return lerp(d0,d1,fp.z);
}