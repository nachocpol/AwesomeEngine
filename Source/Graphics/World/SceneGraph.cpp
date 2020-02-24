#include "SceneGraph.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "LightComponent.h"
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

	mRoot->UpdateLate();
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

Actor* World::SceneGraph::GetRoot() const
{
	return mRoot;
}