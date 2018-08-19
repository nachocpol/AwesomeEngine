#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Graphics/AssetImporter.h"
#include <vector>

namespace Graphics
{
	struct ShadingInfo
	{
		ShadingInfo():
			AlbedoTexture(InvalidTexture),
			BumpMapTexture(InvalidTexture)
		{
		}
		TextureHandle AlbedoTexture;
		TextureHandle BumpMapTexture;
		glm::vec4 AlbedoColor;
	};
	struct Actor
	{
		Mesh AMesh;
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation;

		ShadingInfo ShadeInfo;
	};
	struct Light
	{
		glm::vec3 Position; // For directional, this is direction 
		enum LightType
		{
			Directional,
			Point,
			Spot,
		}Type;
		glm::vec3 Color;
		float Range;	// Only for spot and point
		float Angle;	// Only for Spot
	};

	class Scene
	{
	public:
		Scene(GraphicsInterface* graphics, Graphics::AssetImporter* assetImp);
		~Scene();
		virtual bool Initialize();
		virtual void Update(float dt);
		virtual void Draw(float dt);
		virtual void Resize(int w, int h);
		Actor* AddActor();
		Light* AddLight();

	protected:
		GraphicsInterface* mGraphics;
		AssetImporter* mAssetImporter;
		std::vector<Actor*> mActors;
		std::vector<Light*> mLights;
	};
}