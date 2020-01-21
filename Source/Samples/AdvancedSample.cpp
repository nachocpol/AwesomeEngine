#define NOMINMAX

#include "Graphics/Platform/BaseWindow.h"
#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/World/Model.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/World/Actor.h"

#include "glm/ext.hpp"

#include <stdio.h>

using namespace World;

struct VertexCube
{
	float x, y, z;
};

struct VertexScreen
{
	float x, y, z;
};

struct AppData
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
	glm::vec4 DebugColor;
}AppData;

class AdvancedApp : public AppBase
{
public:
	AdvancedApp() {}
	~AdvancedApp() {}
	void Init();
	void Update();
	void Release();

private:
	Graphics::BufferHandle vertexBuffer;
	Graphics::GraphicsPipeline pipeline;
	Graphics::GraphicsPipeline fullScreenPipeline;
	Graphics::BufferHandle appDataBuffer;
	Graphics::BufferHandle fullScreenBuffer;

	Graphics::Model* mCube;

	Graphics::TextureHandle mainTarget;
	Graphics::TextureHandle mainDepth;

	World::SceneGraph mScene;
};

void AdvancedApp::Init()
{
	AppBase::Init();

	int width = 1920;
	int height = 1080;

	auto colFlags = Graphics::TextureFlags::RenderTarget;
	mainTarget = mGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::RGBA_8_Unorm, colFlags);

	auto depthFlags = Graphics::TextureFlags::DepthStencil;
	mainDepth = mGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::Depth24_Stencil8, depthFlags);

	VertexCube arr[36] =
	{
		-1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f,    // Front
		-1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f, -1.0f,-1.0f, 1.0f,

		-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  // Top
		-1.0f, 1.0f, 1.0f,  1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,

		-1.0f,-1.0f, 1.0f,   1.0f,-1.0f,1.0f,   -1.0f,-1.0f,-1.0f,  // Bot
		 1.0f, -1.0f, 1.0f,  1.0f,-1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,

		1.0f, 1.0f,-1.0f, -1.0f, 1.0f,-1.0f,   -1.0f,-1.0f,-1.0f,    // Back
		1.0f, 1.0f,-1.0f,  -1.0f,-1.0f, -1.0f,  1.0f,-1.0f,-1.0f,

		1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,  1.0f,-1.0f,-1.0f, // Right
		1.0f, 1.0f, 1.0f,  1.0f,-1.0f,-1.0f,  1.0f,-1.0f, 1.0f, 

		-1.0f, 1.0f,-1.0f,  -1.0f, 1.0f,1.0f,  -1.0f,-1.0f,1.0f, // Left
		-1.0f, 1.0f,-1.0f,  -1.0f,-1.0f,1.0f,  -1.0f,-1.0f,-1.0f
	};
	vertexBuffer = mGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::CPUAccess::None, sizeof(arr), &arr[0]);
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSFordwardSimple";
		pdesc.PixelShader.ShaderPath = "Fordward.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSFordwardSimple";
		pdesc.VertexShader.ShaderPath = "Fordward.hlsl";
		pdesc.VertexShader.Type = Graphics::ShaderType::Vertex;

		//Graphics::VertexInputDescription::VertexInputElement eles[1];
		//eles[0].Semantic = "POSITION";
		//eles[0].Idx = 0;
		//eles[0].EleFormat = Graphics::Format::RGB_32_Float;
		//eles[0].Offset = 0;

		Graphics::VertexInputDescription::VertexInputElement eles[4] =
		{
			 {"POSITION", 0, Graphics::Format::RGB_32_Float, 0}
			,{"NORMAL", 0, Graphics::Format::RGB_32_Float, 12}
			,{"TANGENT", 0,	Graphics::Format::RGB_32_Float, 24}
			,{"TEXCOORD", 0, Graphics::Format::RG_32_Float, 36}
		};

		pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
		pdesc.VertexDescription.Elements = eles;
		pdesc.DepthEnabled = true;
		pdesc.DepthFunction = Graphics::LessEqual;
		pdesc.DepthFormat = Graphics::Depth24_Stencil8;
		pdesc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
		pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		pipeline = mGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.DepthEnabled = false;
		desc.DepthFunction = Graphics::Always;
		desc.VertexShader.ShaderEntryPoint = "VSFullScreen";
		desc.VertexShader.ShaderPath = "Common.hlsl";
		desc.VertexShader.Type = Graphics::ShaderType::Vertex;
		desc.PixelShader.ShaderEntryPoint = "PSFullScreen";
		desc.PixelShader.ShaderPath = "Common.hlsl";
		desc.PixelShader.Type = Graphics::ShaderType::Pixel;

		Graphics::VertexInputDescription::VertexInputElement eles[1] =
		{
			"POSITION",0, Graphics::Format::RGB_32_Float,0
		};
		desc.VertexDescription.NumElements = 1;
		desc.VertexDescription.Elements = eles;

		desc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
		desc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		fullScreenPipeline = mGraphicsInterface->CreateGraphicsPipeline(desc);

		VertexScreen vtxData[6] =
		{
			-1.0f, 1.0f,0.0f,
			1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,

			-1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
		};
		fullScreenBuffer = mGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::None, sizeof(VertexScreen) * 6, &vtxData);
	}
	appDataBuffer = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));

	mCube = Graphics::ModelFactory::Get()->LoadFromFile("Meshes\\cube.obj", mGraphicsInterface);


	// Spawn some stuff
	for (uint32_t x = 0; x < 8; ++x)
	{
		for (uint32_t y = 0; y < 8; ++y)
		{
			World::Actor* curCube = mScene.SpawnActor();
			curCube->SetPosition((float)x * 2.0f, (float)y * 2.0f, 0.0f);
			curCube->SetModel(mCube);
		}
	}

	mGraphicsInterface->FlushAndWait();
}

void AdvancedApp::Update()
{
	AppBase::Update();

	mGraphicsInterface->SetScissor(0.0f, 0.0f, mWindow->GetWidth(), mWindow->GetHeight());

	// Render to screen buffer
	mGraphicsInterface->SetTargets(1, &mainTarget, &mainDepth);
	float clear[4] = { 0.0f,0.0f,0.0f,1.0f };
	mGraphicsInterface->ClearTargets(1, &mainTarget, clear, &mainDepth, 1.0f, 0);
	{
		mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		mGraphicsInterface->SetGraphicsPipeline(pipeline);
		AppData.View = glm::lookAt(glm::vec3(0.0f, 0.0f, -8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		AppData.Projection = glm::perspective(glm::radians(90.0f), 1920.0f / 1080.0f, 0.1f, 200.0f);
		//{
		//
		//	AppData.Model = glm::mat4(1.0f);
		//	AppData.Model = glm::translate(AppData.Model, glm::vec3(0.0f, 0.0f, 0.0f));
		//	AppData.Model = glm::rotate(AppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		//	AppData.Model = glm::rotate(AppData.Model, TotalTime * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		//	AppData.Model = glm::rotate(AppData.Model, TotalTime* 2.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		//	AppData.Model = glm::scale(AppData.Model, glm::vec3(1.0f, 1.0f, 1.0f));
		//	AppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		//}
		//mGraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
		//mGraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(VertexCube) * 36, sizeof(VertexCube));
		//mGraphicsInterface->Draw(36, 0);

		for (uint32_t actorIdx = 0; actorIdx < mScene.mRoot->GetNumChilds(); ++actorIdx)
		{
			Actor* cur = mScene.mRoot->GetChild(actorIdx);

			AppData.Model = glm::mat4(1.0f);
			AppData.Model = glm::translate(AppData.Model, cur->GetPosition());
			AppData.Model = glm::rotate(AppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			AppData.Model = glm::rotate(AppData.Model, TotalTime * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
			AppData.Model = glm::rotate(AppData.Model, TotalTime* 2.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			AppData.Model = glm::scale(AppData.Model, glm::vec3(1.0f, 1.0f, 1.0f));
			AppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			
			mGraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
			mGraphicsInterface->SetVertexBuffer(mCube->Meshes[0].VertexBuffer, mCube->Meshes[0].VertexSize *mCube->Meshes[0].NumVertex, mCube->Meshes[0].VertexSize);
			mGraphicsInterface->SetIndexBuffer(mCube->Meshes[0].IndexBuffer, mCube->Meshes[0].NumIndices * sizeof(uint32_t), Graphics::Format::R_32_Uint);
			mGraphicsInterface->DrawIndexed(mCube->Meshes[0].NumIndices);
		}
	}
	mGraphicsInterface->DisableAllTargets();

	// Output to the screen
	mGraphicsInterface->SetGraphicsPipeline(fullScreenPipeline);
	mGraphicsInterface->SetVertexBuffer(fullScreenBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
	mGraphicsInterface->SetResource(mainTarget, 0);
	mGraphicsInterface->Draw(6, 0);
}

void AdvancedApp::Release()
{

	AppBase::Release();
}

AdvancedApp app;
ENTRY_POINT(app, "Advanced App", false);

