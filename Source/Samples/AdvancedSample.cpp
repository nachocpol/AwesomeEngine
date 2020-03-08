#define NOMINMAX

#include "Graphics/Platform/BaseWindow.h"
#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/World/Model.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/World/Actor.h"
#include "Graphics/World/CameraComponent.h"
#include "Graphics/World/LightComponent.h"
#include "Graphics/World/PhysicsWorld.h"
#include "Graphics/TestRenderer.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "Graphics/World/TransformComponent.h"

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
	Actor* mCamera;

	World::Renderable* sun;
	World::Renderable* earth;
	World::Renderable* moon;
};

void AdvancedApp::Init()
{
	AppBase::Init();
	mScene.Initialize();
	mRenderer.Initialize(this);

	// Spawn some stuff
	mCube = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/cube.obj", mGraphicsInterface);
	for (uint32_t x = 0; x < 7; ++x)
	{
		for (uint32_t y = 0; y < 7; ++y)
		{
			/*
			// Add a cube:
			Actor* curCube = mScene.SpawnActor();

			TransformComponent* curTransform = curCube->AddComponent<TransformComponent>();
			glm::vec3 curPos = glm::vec3(((float)x * 2.0f) - 7.0f, 4.0f, ((float)y * 2.0f) - 7.0f);
			curTransform->SetPosition(curPos);
			curTransform->SetRotation(glm::vec3(x+1,x,y+1));

			ModelComponent* modelComponent = curCube->AddComponent<ModelComponent>();
			modelComponent->SetModel(mCube);

			RigidBodyComponent* rbComp = curCube->AddComponent<RigidBodyComponent>();

			rbComp->AddCollider(curCube->AddComponent<BoxColliderComponent>());
			*/
			// Add a light:
			//Actor* curLight = mScene.SpawnActor();
			//
			//TransformComponent* lightTrans = curLight->AddComponent<TransformComponent>();
			//lightTrans->SetPosition(curPos + glm::vec3(0.0f,glm::linearRand(1.0f,2.0f),0.0f));
			//
			//LightComponent* pointComp = curLight->AddComponent<LightComponent>();
			//pointComp->SetLightType(LightComponent::LightType::Point);
			//pointComp->SetColor(glm::vec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f)));
			//pointComp->SetIntensity(glm::linearRand(0.5f, 2.0f));
			//pointComp->SetRadius(glm::linearRand(1.0f, 5.5f));
		}
	}	

	//Actor* sceneLight = mScene.SpawnActor();
	//TransformComponent* lightTrans = sceneLight->AddComponent<TransformComponent>();
	//lightTrans->SetPosition(0.0f, 2.0f, 0.0f);
	//LightComponent* lightComp = sceneLight->AddComponent<LightComponent>();
	//lightComp->SetRadius(20.0f);
	//lightComp->SetColor(glm::vec3(0.8f, 0.8f, 0.95f));

	// Ground
	{
		/*
		Actor* ground = mScene.SpawnActor();
		
		TransformComponent* groundTransform = ground->AddComponent<TransformComponent>();
		groundTransform->SetScale(20.0f, 0.1f, 20.0f);
		groundTransform->SetPosition(0.0f, -3.5f, 0.0f);

		ModelComponent* groundModel = ground->AddComponent<ModelComponent>();
		groundModel->SetModel(mCube);

		RigidBodyComponent* groudRb = ground->AddComponent<RigidBodyComponent>();
		groudRb->SetBodyType(RigidBodyComponent::Type::Static);

		BoxColliderComponent* boxCol = ground->AddComponent<BoxColliderComponent>();
		boxCol->SetLocalExtents(glm::vec3(10.0f, 0.05f, 10.0f));
		groudRb->AddCollider(boxCol);
		*/
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

	/*
	mCamera = mScene.SpawnActor();
	TransformComponent* camTransform = mCamera->AddComponent<TransformComponent>();
	camTransform->SetPosition(glm::vec3(-12.0f, 1.0f, 0.0f));

	CameraComponent* camComponent = mCamera->AddComponent<CameraComponent>();
	camComponent->ConfigureProjection(
		(float)mWindow->GetWidth() / (float)mWindow->GetHeight(), 75.0f, 0.1f, 40.0f
	);
	*/
	
	mGraphicsInterface->FlushAndWait();
}

void AdvancedApp::Update()
{
	AppBase::Update();

	//sun->Rotate(0.0f, 0.0005f * DeltaTime, 0.0f);
	//earth->Rotate(0.0f, 0.001f * DeltaTime, 0.0f);

	mScene.UpdatePhysics(DeltaTime);
	mScene.Update(DeltaTime);
	mRenderer.Render(&mScene);
}

void AdvancedApp::Release()
{
	AppBase::Release();
}

AdvancedApp app;
ENTRY_POINT(app, "Advanced App", false);

