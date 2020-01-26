#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Graphics/AssetImporter.h"
#include <vector>

namespace World
{
	class Renderable;
	class Actor;
	class SceneGraph
	{
	public:
		SceneGraph();
		~SceneGraph();

		void Update(float deltaTime);
		Renderable* SpawnRenderable(Actor* parent = nullptr);

	//private:
		Actor* mRoot;
	};
}