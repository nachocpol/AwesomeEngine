#include "SceneGraph.h"
#include "Actor.h"

using namespace World;

SceneGraph::SceneGraph()
{
	mRoot = new Actor;
}

SceneGraph::~SceneGraph()
{
}

Actor* SceneGraph::SpawnActor()
{
	Actor* actor = new Actor;
	mRoot->AddChild(actor);
	return actor;
}
