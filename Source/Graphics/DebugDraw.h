#pragma once

#include "GraphicsInterface.h"
#include "Declarations.h"

#include "glm/glm.hpp"

#include <vector>

namespace World
{
	class Camera;
}

#define MAX_LINES 1024

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

		void StartFrame();
		void Flush(World::Camera* camera);
		void EndFrame();
		
		void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0f));

	private:
		GraphicsInterface* mGraphicsInterface;
		GraphicsPipeline mDebugPipeline;

		CameraData mCameraData;
		BufferHandle mCameraDataCb;

		struct LineItem
		{
			LineItem(glm::vec3 start, glm::vec3 end, glm::vec4 color);
			glm::vec3 Start;
			glm::vec3 End;
			glm::vec4 Color;
		};
		std::vector<LineItem> mLines;
		BufferHandle mLinesVtxBuffer;
	};
}