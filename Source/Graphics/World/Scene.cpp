#include "Scene.h"

namespace Graphics
{
	Scene::Scene(GraphicsInterface* graphics):
		mGraphics(graphics)
	{
	}

	Scene::~Scene()
	{
	}

	bool Scene::Initialize()
	{
		return false;
	}

	void Scene::Update(float dt)
	{
	}

	void Scene::Draw(float dt)
	{
	}

	Actor* Scene::AddActor()
	{
		Actor* a = new Actor;
		mActors.push_back(a);
		return a;
	}
}