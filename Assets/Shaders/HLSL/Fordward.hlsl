cbuffer AppData : register(b0)
{
	float4x4 Model;
	float4x4 View;
	float4x4 Projection;
	float4 DebugColor;
}

struct VSIn
{
	float3 Position : POSITION;
};

struct VSOut
{
	float4 ClipPos : SV_Position;
};

VSOut VSFordwardSimple(VSIn i)
{
	VSOut o;
	o.ClipPos = mul(Projection,mul(View,mul(Model,float4(i.Position,1.0))));
	return o;
}

float4 PSFordwardSimple(VSOut i): SV_Target0
{
	//return float4(1.0f,0.0f,0.0f,1.0f);
	return DebugColor;
}