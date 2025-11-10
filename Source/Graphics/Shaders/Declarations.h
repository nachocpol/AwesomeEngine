/*
	Declarations.h
*/
#pragma once

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include "ShaderDeclarationMacros.h"

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

#endif