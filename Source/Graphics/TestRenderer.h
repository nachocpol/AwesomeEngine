#pragma once

#include "RendererBase.h"
#include "GraphicsInterface.h"

namespace Graphics
{
	class TestRenderer : public RendererBase
	{
	public:
		TestRenderer();
		~TestRenderer();
		void Initialize(AppBase* app);
		void Release();
		void Render(World::SceneGraph* scene)override;

	private:
		Graphics::TextureHandle mColourRt;
		Graphics::TextureHandle mDepthRt;
		Graphics::GraphicsPipeline mTestPipeline;
		Graphics::GraphicsPipeline mPresentPipeline;
		Graphics::BufferHandle mPresentVtxBuffer;
		struct AppData
		{
			glm::mat4 Model;
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec4 DebugColor;
		}mAppData;
		Graphics::BufferHandle mAppDataCB;
	};
}