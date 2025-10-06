#include "..\ShaderDeclarationMacros.h"
#include "..\Samplers.hlsl"

struct VertexData
{
    float4 m_Position;
    float2 m_UV;
    float2 PAD;
};

STRUCTUREDBUFFER(g_VertexBuffer, VertexData, 0);
TEXTURE_2D(g_TestTexture, 1);

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
    float2 UV  : UV0;
};

VSOut VSMain(uint vid : SV_VertexID)
{
    const VertexData vertexData = g_VertexBuffer[vid];
    
    float3 modelPos = vertexData.m_Position.xyz;

    VSOut vsOut;
    
    vsOut.POS = mul(gViewerConstants.ViewProjection, mul(gViewerConstants.Model, float4(modelPos, 1.0)));
    
    vsOut.COL.r = rand1(vid / 250);
    vsOut.COL.g = rand1((vid + 200) / 250);
    vsOut.COL.b = rand1((vid + 600) / 250);

    vsOut.UV = vertexData.m_UV;

    return vsOut;
}

struct PSInputs
{
    float4 POS : SV_Position;
    float3 COL : COLOR;
    float2 UV  : UV0;
};

float4 PSMain(PSInputs inputs) : SV_Target0
{
    float3 textureData = g_TestTexture.Sample(LinearWrapSampler, inputs.UV).rgb;

    return float4(textureData, 1.0);
}

#endif