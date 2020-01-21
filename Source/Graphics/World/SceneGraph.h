#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Graphics/AssetImporter.h"
#include <vector>

namespace World
{
	class Actor;
	class SceneGraph
	{
	public:
		SceneGraph();
		~SceneGraph();

		Actor* SpawnActor();

	//private:
		Actor* mRoot;
	};
}