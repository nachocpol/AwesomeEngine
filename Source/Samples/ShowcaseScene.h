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
	struct Camera
	{
		Camera(): Position(0.0f,10.0f,-1.0f),
			View(0.0f,0.0f,1.0f),
			Up(0.0f,1.0f,0.0f),
			Right(1.0f,0.0,0.0f),
			LastMouse(512.0f,420.0f),
			Yaw(-90.0f),
			Pitch(0.0f)
		{
		}
		glm::vec3 Position;
		glm::vec3 View;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec2 LastMouse;
		float Yaw;
		float Pitch;
	}mCamera;

	struct AppData
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
		glm::vec4 DebugColor;
	}mAppData;
	Graphics::BufferHandle mAppDataHandle;

	struct MaterialInfo
	{
		glm::vec4 AlbedoColor;
		int UseBumpTex;
	}mMaterialInfo;
	Graphics::BufferHandle mMaterialInfoHandle;

	struct LightInfo
	{
		glm::vec4 LightPosition; // xyz(For directional this is direction) w(type)
		glm::vec4 LightColor;	 // w not used
		float LightRange;		 // Only for spot and point
		float LightAngle;		 // Only for Spot
	}mLightInfo;
	Graphics::BufferHandle mLightInfoHandle;

	// Pipeline used to fill the GBuffer
	Graphics::GraphicsPipeline mGBufferPipeline;
	struct GBuffer
	{
		Graphics::TextureHandle Color;
		Graphics::TextureHandle Normals;
		Graphics::TextureHandle Position;
		Graphics::TextureHandle Depth;
	}mGBuffer;

	// Light pass
	Graphics::GraphicsPipeline mLightPassPipeline;
	Graphics::TextureHandle mLightPass;
	
	Graphics::GraphicsPipeline mFullScreenPipeline;
	Graphics::BufferHandle mFullScreenQuad;

	Graphics::TextureHandle mWhiteTexture;

	// Atmosphere
	struct AtmosphereData
	{
		glm::vec4 View;
	}mAtmosphereData;
	Graphics::BufferHandle mAtmosphereDataHandle;

	Graphics::GraphicsPipeline mAtmospherePipeline;
};