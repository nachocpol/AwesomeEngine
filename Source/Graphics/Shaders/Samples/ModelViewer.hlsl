#include "..\ShaderDeclarationMacros.h"

STRUCTUREDBUFFER(g_Positions, float4, 0);

CBUFFER(ViewerConstants)
	float4x4 Model;
	float4x4 ViewProjection;
CBUFFER_END(ViewerConstants, 0)

#if !defined(__cplusplus)

float rand1(float n)  { return frac(sin(n) * 43758.5453123); }

struct VSOut
{
    float4 POS : SV_Position;
    float3 COL : COLOR;
};

VSOut VSMain(uint vid : SV_VertexID)
{
    float3 modelPos = g_Positions[vid].xyz;

    VSOut vsOut;
    
    vsOut.POS = mul(gViewerConstants.ViewProjection, mul(gViewerConstants.Model, float4(modelPos, 1.0)));
    
    vsOut.COL.r = rand1(vid / 250);
    vsOut.COL.g = rand1((vid + 200) / 250);
    vsOut.COL.b = rand1((vid + 600) / 250);

    return vsOut;
}

struct PSInputs
{
    float4 POS : SV_Position;
    float3 COL : COLOR;
};

float4 PSMain(PSInputs inputs) : SV_Target0
{
    return float4(inputs.COL, 1.0);
}

#endif