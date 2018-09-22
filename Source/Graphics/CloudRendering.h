#pragma once

#include "GraphicsInterface.h"
#include "glm/glm.hpp"

namespace Graphics
{
	class CloudRenderer
	{
	public:
		CloudRenderer();
		~CloudRenderer();
		bool Initialize(GraphicsInterface* graphicsInterface);
		void Draw(float dt,glm::vec3 camPos, glm::mat4 iViewProj, glm::vec3 SunDirection);
		void ShowDebug();

	private:
		TextureHandle mCloudCoverage;
		TextureHandle mTestTexture3D;
		GraphicsInterface* mGraphicsInterface;
		GraphicsPipeline mCloudsPipeline;

		struct CloudsData
		{
			glm::vec4 ViewPosition;
			glm::mat4 InvViewProj;
			glm::vec4 SunDirection;
			float CloudBase = 150.0f;
			float CloudExtents = 100.0f;
			float Absorption = 0.25f;
		}mCloudsData;
		BufferHandle mCloudsDataHandle;
	};
	
}