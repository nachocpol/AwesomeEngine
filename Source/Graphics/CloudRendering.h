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
		GraphicsPipeline mCloudsCompositePipeline;

		float mDownScaleFactor = 0.5f;

		ComputePipeline mCloudsPipelineCompute;
		TextureHandle mCloudsIntermediate;
		glm::vec2 mCurRenderSize;

		ComputePipeline mCloudShadowPipeline;
		TextureHandle mCloudShadowTexture;

		struct CloudsData
		{
			glm::vec4 ViewPosition;
			glm::mat4 InvViewProj;
			glm::vec4 SunDirection;
			float CloudBase = 240.0f;
			float CloudExtents = 100.0f;
			float Absorption = 0.55f;
			float CloudsScale = 0.00015f;
			float EarthR = 6360e3f;
			float Time = 0.0f;
		}mCloudsData;
		BufferHandle mCloudsDataHandle;

		TextureHandle mTestWorley;
	};
	
}