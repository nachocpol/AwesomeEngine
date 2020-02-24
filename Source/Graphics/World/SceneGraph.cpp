#include "SceneGraph.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "TransformComponent.h"
#include "PhysicsWorld.h"

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
	// Physics representation of this scene graph:
	mPhysicsWorld = new PhysicsWorld;
	mPhysicsWorld->Initialize();
	
	// Add root entity:
	mRoot = new Actor;
	mRoot->AddComponent<TransformComponent>();
	mRoot->mSceneOwner = this;
}

void SceneGraph::UpdatePhysics(float deltaTime)
{
	// Update physics world:
	mPhysicsWorld->Update(deltaTime);

	// Then bring both in sync:
	mRoot->UpdatePhysics();
}

void SceneGraph::Update(float deltaTime)
{
	// Generic component update, figure world transformations etc.
	mRoot->Update(deltaTime);

	// SceneGraph -> PhysicsWorld sync back. Also update things that need position/transforms to be ready.
	mRoot->UpdateLate();
}

Actor* SceneGraph::SpawnActor(Actor* parent)
{
	Actor* actor = new Actor;
	actor->mSceneOwner = this;
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

PhysicsWorld* SceneGraph::GetPhysicsWorld() const
{
	return mPhysicsWorld;
}
