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
	};
	struct Actor
	{
		Mesh AMesh;
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation;

		ShadingInfo ShadeInfo;
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

	protected:
		GraphicsInterface* mGraphics;
		AssetImporter* mAssetImporter;
		std::vector<Actor*> mActors;
	};
}