#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Graphics/AssetImporter.h"
#include <vector>

namespace World
{
	class Renderable;
	class Camera;
	class Actor;
	class Light;
	class SceneGraph
	{
	public:
		SceneGraph();
		~SceneGraph();

		void Update(float deltaTime);
		Renderable* SpawnRenderable(Actor* parent = nullptr);
		Camera* SpawnCamera(Actor* parent = nullptr);
		Light* SpawnLight(Actor* parent = nullptr);

		Actor* GetRoot()const;
		const std::vector<Camera*>& GetCameras()const;
		const std::vector<Light*>& GetLights()const;

	private:
		Actor* mRoot;
		std::vector<Camera*> mCameras;
		std::vector<Light*> mLights;
	};
}