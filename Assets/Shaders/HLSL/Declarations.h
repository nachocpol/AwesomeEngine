/*
	Declarations.h
*/

#if defined(__cplusplus)
	#pragma once
	#include "glm/glm.hpp"

	//#define REGISTER(t,s) static const uint32_t 
	#define CBUFFER(name, slot)static const unsigned int k##name##Slot = slot; struct name

	#define float4x4	glm::mat4
	#define float4		glm::vec4
	#define float3		glm::vec3
	#define float2		glm::vec2
#else
	//#define REGISTER(r) : register(r)
	//#define CBUFFER(name) cbuffer name
	#define CBUFFER(name, slot) cbuffer name : register(b##slot)
#endif

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
};

CBUFFER(LightData, 2)
{
	float LightType;			// Should match Light::LightType::T 
	float3 LightColor;
	float3 LightPosDirection;
	float LightIntensity;
	float LightRadius;
};

#if defined(__cplusplus)
	#undef float4x4
	#undef float4
	#undef float2
	#undef float3
#endif