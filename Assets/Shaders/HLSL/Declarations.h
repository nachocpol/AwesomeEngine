/*
	Declarations.h
*/
#pragma once

#include "ShaderDeclarationMacros.h"

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

#include "ShaderDeclarationMacrosEnd.h"