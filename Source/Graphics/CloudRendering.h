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
		void Draw(float dt,glm::vec3 camPos, glm::mat4 iViewProj);
		void ShowDebug();

	private:
		TextureHandle mTestTexture;
		TextureHandle mTestTexture3D;
		GraphicsInterface* mGraphicsInterface;
		GraphicsPipeline mCloudsPipeline;

		struct CloudsData
		{
			glm::vec4 ViewPosition;
			glm::mat4 InvViewProj;
		}mCloudsData;
		BufferHandle mCloudsDataHandle;
	};
	
}