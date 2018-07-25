#pragma once

#include "Graphics/World/Scene.h"
#include "Graphics/AssetImporter.h"

class ShowcaseScene : public Graphics::Scene
{
public:
	ShowcaseScene(Graphics::GraphicsInterface* graphics,Graphics::AssetImporter* assetImp);
	~ShowcaseScene();
	bool Initialize()override final;
	void Update(float dt)override final;
	void Draw(float dt)override final;
	void Resize(int w, int h)override final;

private:

	Graphics::GraphicsPipeline mFordwardPipeline;
	struct AppData
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
		glm::vec4 DebugColor;
	}mAppData;
	Graphics::BufferHandle mAppDataHandle;

	Graphics::TextureHandle mMainTarget;
	Graphics::TextureHandle mMainDepthTarget;

	Graphics::GraphicsPipeline mFullScreenPipeline;
	Graphics::BufferHandle mFullScreenQuad;

	Graphics::TextureHandle mTestAlbedo;
	Graphics::TextureHandle mTestBump;
};