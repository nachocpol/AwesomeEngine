/*
	Declarations.h
*/

#if defined(__cplusplus)
	#pragma once
	#include "glm/glm.hpp"

	#define CBUFFER(name, slot)static const unsigned int k##name##Slot = slot; struct name
	#define STRUCTUREDBUFFER(name, type, slot) static const unsigned int k##name##Slot = slot; static const unsigned int k##name##Stride = sizeof(type);

	#define float4x4	glm::mat4
	#define float4		glm::vec4
	#define float3		glm::vec3
	#define float2		glm::vec2

	namespace Declarations
	{
#else
	#define CBUFFER(name, slot) cbuffer name : register(b##slot)
#define STRUCTUREDBUFFER(name, type, slot) StructuredBuffer<type> name : register(t##slot); RWStructuredBuffer<type> RW##name : register(u##slot);
#endif

struct Light
{
	float Type; // Should match Light::LightType::T 
	float3 Color;
	float3 PosDirection;
	float Intensity;
	float Radius;
};
STRUCTUREDBUFFER(Lights, Light, 0);

CBUFFER(UIData, 0)
{
	float4x4 ProjectionUI;	// ImGUI ortho projection
};

CBUFFER(CameraData, 0)
{
	float4x4 InvViewProj;
};

CBUFFER(ItemData, 1)
{
	float4x4 World;
	float4 DebugColor;
	int NumLights;
};

#if defined(__cplusplus)
	} // End namespace
	#undef float4x4
	#undef float4
	#undef float2
	#undef float3
#endif