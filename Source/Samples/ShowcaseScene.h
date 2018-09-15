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
		Camera(): Position(0.0f,1.0f,-1.0f),
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
		glm::mat4 ModelMatrix;
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
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
		glm::mat4 ViewMatrix;
		glm::mat4 InvViewProj;
		glm::vec4 ViewPosition;
		glm::vec3 SunDirection;
		float EarthR = 6360e3f;
		float AtmosR = 6420e3f;
		float Hr = 7994.0f;
		float Hm = 1200.0f;
		float SunIntensity = 20.0f;
		glm::vec3 BetaR = glm::vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
		float Pad0 = 0.0f;
		glm::vec3 BetaM = glm::vec3(21e-6f, 21e-6f, 21e-6f);
		float Pad1 = 0.0f;
	}mAtmosphereData;
	Graphics::BufferHandle mAtmosphereDataHandle;

	Graphics::GraphicsPipeline mAtmospherePipeline;
};