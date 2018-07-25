#include "Scene.h"

namespace Graphics
{
	Scene::Scene(GraphicsInterface* graphics, AssetImporter* assetImp):
		mGraphics(graphics),
		mAssetImporter(assetImp)
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

	void Scene::Resize(int w, int h)
	{
	}

	Actor* Scene::AddActor()
	{
		Actor* a = new Actor;
		mActors.push_back(a);
		return a;
	}
}