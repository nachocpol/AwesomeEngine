/*
	Declarations.h
*/

#if defined(__cplusplus)
	#include "glm/glm.hpp"

	#define REGISTER(r)
	#define CBUFFER(name) struct name

	#define float4x4	glm::mat4
	#define float4		glm::vec4
	#define float3		glm::vec3
	#define float2		glm::vec2
#else
	#define REGISTER(r) : register(r)
	#define CBUFFER(name) cbuffer name
#endif

CBUFFER(UIData) REGISTER(b0)
{
	float4x4 ProjectionUI;	// ImGUI ortho projection
};

CBUFFER(CameraData) REGISTER(b0)
{
	float4x4 InvViewProj;
};

CBUFFER(ItemData) REGISTER(b1)
{
	float4x4 World;
};