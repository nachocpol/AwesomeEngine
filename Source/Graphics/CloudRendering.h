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
		void DestroyTextures();
		void CreateTextures();

		TextureHandle mCloudCoverage;
		TextureHandle mBaseNoise;
		TextureHandle mDetailNoise;
		GraphicsInterface* mGraphicsInterface;
		GraphicsPipeline mCloudsPipeline;

		ComputePipeline mCloudsPipelineCompute;

		struct CloudsData
		{
			glm::vec4 ViewPosition;
			glm::mat4 InvViewProj;
			glm::vec4 SunDirection;
			float CloudBase = 150.0f;
			float CloudExtents = 100.0f;
			float Absorption = 0.55f;
			float CoverageScale = 0.0002f;
			float BaseNoiseScale = 0.0005f;
			float DetailNoiseScale = 0.0005f;
		}mCloudsData;
		BufferHandle mCloudsDataHandle;
	};
	
}