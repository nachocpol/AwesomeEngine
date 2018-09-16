/*
	Utils.hlsl
	A bunch of utility methods like tonemapping, intersections etc.
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