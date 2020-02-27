// SceneGraph.h
// A SceneGraph is a virtual representation of the 3D world. It is used as a middle man
// between the user and the rendering and physics engines. You use the scene graph to 
// spawn the different actors into the world.
// You should update the scene graph once every frame before passing it to the 
// renderer.
#pragma once

#include "Graphics/GraphicsInterface.h"
#include "Graphics/AssetImporter.h"
#include <vector>

namespace reactphysics3d
{
	class DynamicsWorld;
}

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

		void Initialize();
		void UpdatePhysics(float deltaTime);
		void Update(float deltaTime);
		
		Actor* SpawnActor(Actor* parent = nullptr);

		Actor* GetRoot()const;

	private:
		Actor* mRoot;
	};
}