
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