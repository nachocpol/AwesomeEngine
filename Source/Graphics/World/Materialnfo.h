#pragma once

#include "glm/glm.hpp"

struct MaterialInfo
{
	MaterialInfo()
		:BaseColor(1.0f,0.0f,0.0f)
		,Metalness(0.0f)
		,Roughness(0.5f)
	{
	}
	glm::vec3 BaseColor;
	float Metalness;
	float Roughness;
};