/*
	Declarations.hlsl
*/
cbuffer UIData : register(b0)
{
	float4x4 ProjectionUI;	// ImGUI ortho projection
}

cbuffer CameraData : register(b0)
{
	float4x4 InvViewProj;
}

cbuffer ItemData : register(b1)
{
	float4x4 World;
}