#pragma once

#include "glm/glm.hpp"

namespace Graphics
{
	struct CameraData
	{
		glm::mat4 InvViewProj;
	};
	
	struct ItemData
	{
		glm::mat4 World;
	};
}