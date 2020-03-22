/*
	Declarations.h
*/

#if defined(__cplusplus)
	#pragma once
	#include "glm/glm.hpp"

	#define CBUFFER(name) struct name {
	#define CBUFFER_END(name, slot) }; static const unsigned int k##name##Slot = slot;
	#define STRUCTUREDBUFFER(name, type, slot) static const unsigned int k##name##Slot = slot; static const unsigned int k##name##Stride = sizeof(type);

	#define float4x4	glm::mat4
	#define float4		glm::vec4
	#define float3		glm::vec3
	#define float2		glm::vec2

	namespace Declarations
	{
#else
	#define CBUFFER(name) struct name {
#define CBUFFER_END(name, slot) }; cbuffer name##CB : register(b##slot) { name g##name; }; 
	#define STRUCTUREDBUFFER(name, type, slot) StructuredBuffer<type> name : register(t##slot); RWStructuredBuffer<type> RW##name : register(u##slot);
#endif

struct Light
{
	int Type; // Should match Light::LightType::T 
	float3 Color;
	float3 PosDirection;
	float Intensity;
	float Radius;
};
STRUCTUREDBUFFER(Lights, Light, 0);

struct Tile
{
	int Lights[32]; // kMaxLightsPerTile
	int NumLights;
};
STRUCTUREDBUFFER(Tiles, Tile, 1);

CBUFFER(UIData)
	float4x4 ProjectionUI;	// ImGUI ortho projection
CBUFFER_END(UIData, 0)

CBUFFER(CameraData)
	float4x4 InvViewProj;
	float3 CameraWorldPos;
CBUFFER_END(CameraData, 0)

CBUFFER(ItemData)
	float4x4 World;
	float4 DebugColor;
	int NumLights;
	float3 BaseColor;
	float Metalness;
	float Roughness;
CBUFFER_END(ItemData, 1)

CBUFFER(DebugData)
	int DebugCubemap;
	int Equirectangular;
CBUFFER_END(DebugData, 2)

CBUFFER(IBLData)
	float Roughness;
CBUFFER_END(IBLData, 0)

#if defined(__cplusplus)
	} // End namespace
	#undef float4x4
	#undef float4
	#undef float2
	#undef float3
#endif