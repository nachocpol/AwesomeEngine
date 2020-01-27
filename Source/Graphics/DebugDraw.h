#pragma once

#include "glm/glm.hpp"

namespace Graphics
{
	class GraphicsInterface;
	class DebugDraw
	{
	private:
		DebugDraw();
		DebugDraw(const DebugDraw& other) {};
		~DebugDraw();

	public:
		static DebugDraw* GetInstance();
		void Initialize(GraphicsInterface* graphicsInterface);
		void Release();
		
		void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color);

	private:
		GraphicsInterface* mGraphicsInterface;
	};
}