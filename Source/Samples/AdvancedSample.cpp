#define NOMINMAX

#include "Graphics/Platform/BaseWindow.h"
#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/World/Model.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/World/Actor.h"
#include "Graphics/World/Renderable.h"
#include "Graphics/World/Camera.h"
#include "Graphics/World/Light.h"
#include "Graphics/TestRenderer.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <stdio.h>

using namespace World;

class AdvancedApp : public AppBase
{
public:
	AdvancedApp() {}
	~AdvancedApp() {}
	void Init();
	void Update();
	void Release();

private:
	Graphics::Model* mCube;
	World::SceneGraph mScene;
	Graphics::TestRenderer mRenderer;
	World::Camera* mCamera;

	World::Renderable* sun;
	World::Renderable* earth;
	World::Renderable* moon;
};

void AdvancedApp::Init()
{
	AppBase::Init();

	mRenderer.Initialize(this);

	// Spawn some stuff
	mCube = Graphics::ModelFactory::Get()->LoadFromFile("Meshes\\cube.obj", mGraphicsInterface);
	for (uint32_t x = 0; x < 6; ++x)
	{
		for (uint32_t y = 0; y < 6; ++y)
		{
			World::Renderable* curCube = mScene.SpawnRenderable();
			glm::vec3 curPos = glm::vec3(((float)x * 2.0f) - 6.0f, -2.0f, ((float)y * 2.0f) - 6.0f);
			curCube->SetPosition(curPos);
			curCube->SetRotation(glm::vec3(x+1,x,y+1));
			curCube->SetModel(mCube);

			World::Light* pointLight = mScene.SpawnLight();
			pointLight->SetLightType(World::Light::LightType::Point);
			pointLight->SetPosition(curPos + glm::vec3(0.0f,glm::linearRand(1.0f,2.0f),0.0f));
			pointLight->SetColor(glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f)));
			pointLight->SetIntensity(glm::linearRand(0.5f, 2.0f));
			pointLight->SetRadius(glm::linearRand(1.0f, 2.5f));
		}
	}	

	//sun = mScene.SpawnRenderable();
	//earth = mScene.SpawnRenderable(sun);
	//moon = mScene.SpawnRenderable(earth);
	//
	//sun->SetModel(mCube);
	//earth->SetModel(mCube);
	//earth->SetPosition(glm::vec3(8.0f, 0.0f, 0.0f));
	//moon->SetModel(mCube);
	//moon->SetPosition(glm::vec3(3.0f, 0.0f, 0.0f));

	mCamera = mScene.SpawnCamera();
	mCamera->ConfigureProjection(
		(float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 75.0f, 0.1f, 40.0f
	);

	mGraphicsInterface->FlushAndWait();
}

void AdvancedApp::Update()
{
	AppBase::Update();

	//sun->Rotate(0.0f, 0.0005f * DeltaTime, 0.0f);
	//earth->Rotate(0.0f, 0.001f * DeltaTime, 0.0f);

	mScene.Update(DeltaTime);
	mRenderer.Render(&mScene);
}

void AdvancedApp::Release()
{
	AppBase::Release();
}

AdvancedApp app;
ENTRY_POINT(app, "Advanced App", false);

