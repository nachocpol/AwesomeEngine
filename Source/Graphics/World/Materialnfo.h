#pragma once

#include "glm/glm.hpp"

struct MaterialInfo
{
	MaterialInfo()
		:BaseColor(1.0f,0.0f,1.0f)
		,Metalness(0.0f)
		,Roughness(1.0f)
	{
	}
	glm::vec3 BaseColor;
	float Metalness;
	float Roughness;
};