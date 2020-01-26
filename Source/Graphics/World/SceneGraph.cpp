#include "SceneGraph.h"
#include "Actor.h"
#include "Renderable.h"

using namespace World;

SceneGraph::SceneGraph()
{
	mRoot = new Actor;
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::Update(float deltaTime)
{
	mRoot->Update(deltaTime);
}

Renderable* SceneGraph::SpawnRenderable(Actor* parent /*= nullptr*/)
{
	Renderable* renderable = new Renderable;
	if (parent)
	{
		parent->AddChild(renderable);
	}
	else
	{
		mRoot->AddChild(renderable);
	}
	return renderable;
}
