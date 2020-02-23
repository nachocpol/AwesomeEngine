#include "SceneGraph.h"
#include "Actor.h"
#include "Renderable.h"
#include "Camera.h"
#include "Light.h"
#include "TransformComponent.h"

#include "reactphysics3d.h"

using namespace World;

SceneGraph::SceneGraph():
	mPhysicsWorld(nullptr)
{
}

SceneGraph::~SceneGraph()
{
}

void World::SceneGraph::Initialize()
{
	mRoot = new Actor;
	mRoot->AddComponent<TransformComponent>();

	// Setup physics world:
	reactphysics3d::Vector3 kGravity(0.0f, -9.81f, 0.0f);
	reactphysics3d::WorldSettings kPhysicsSettings = {};
	mPhysicsWorld = new reactphysics3d::DynamicsWorld(kGravity, kPhysicsSettings);

}

void SceneGraph::UpdatePhysics(float deltaTime)
{
	// Start by stepping the physics world. deltaTime is not constant... We probably want to fix that!
	mPhysicsWorld->update(deltaTime / 1000.0f);

	// Now update actors, this will retrieve the values from the physics simulation:
	mRoot->UpdatePhysics();
}

void SceneGraph::Update(float deltaTime)
{
	mRoot->Update(deltaTime);
}

Actor* SceneGraph::SpawnActor(Actor* parent)
{
	Actor* actor = new Actor;
	if (parent)
	{
		parent->AddChild(actor);
	}
	else
	{
		mRoot->AddChild(actor);
	}
	return actor;
}

//Renderable* SceneGraph::SpawnRenderable(Actor* parent /*= nullptr*/)
//{
//	Renderable* renderable = new Renderable;
//	if (parent)
//	{
//		parent->AddChild(renderable);
//	}
//	else
//	{
//		mRoot->AddChild(renderable);
//	}
//	renderable->SetPhysicsWorld(mPhysicsWorld);
//	return renderable;
//}
//
//Camera* SceneGraph::SpawnCamera(Actor* parent /*= nullptr*/)
//{
//	Camera* camera = new Camera;
//	if (parent)
//	{
//		parent->AddChild(camera);
//	}
//	else
//	{
//		mRoot->AddChild(camera);
//	}
//	mCameras.push_back(camera);
//	return camera;
//}
//
//Light* SceneGraph::SpawnLight(Actor* parent /*= nullptr*/)
//{
//	Light* light = new Light;
//	if (parent)
//	{
//		parent->AddChild(light);
//	}
//	else
//	{
//		mRoot->AddChild(light);
//	}
//	mLights.push_back(light);
//	return light;
//}

Actor* World::SceneGraph::GetRoot() const
{
	return mRoot;
}

//const std::vector<Camera*>& SceneGraph::GetCameras() const
//{
//	return mCameras;
//}
//
//const std::vector<Light*>& SceneGraph::GetLights() const
//{
//	return mLights;
//}
