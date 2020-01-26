#include "TestRenderer.h"
#include "World/SceneGraph.h"
#include "World/Renderable.h"
#include "World/Model.h"
#include "Core/App/AppBase.h"
#include "Graphics/Platform/BaseWindow.h"

using namespace Graphics;
using namespace World;

struct VertexScreen
{
	float x, y, z;
};

TestRenderer::TestRenderer()
{
}

TestRenderer::~TestRenderer()
{
}

void TestRenderer::Initialize(AppBase * app)
{
	// Cache some useful stuff:
	mOwnerApp = app;
	mGraphicsInterface = mOwnerApp->GetGraphicsInterface();

	int width = 1920;
	int height = 1080;

	auto colFlags = Graphics::TextureFlags::RenderTarget;
	mColourRt = mGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::RGBA_8_Unorm, colFlags);

	auto depthFlags = Graphics::TextureFlags::DepthStencil;
	mDepthRt = mGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::Depth24_Stencil8, depthFlags);

	// Test pipeline
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSFordwardSimple";
		pdesc.PixelShader.ShaderPath = "Fordward.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSFordwardSimple";
		pdesc.VertexShader.ShaderPath = "Fordward.hlsl";
		pdesc.VertexShader.Type = Graphics::ShaderType::Vertex;

		Graphics::VertexInputDescription::VertexInputElement eles[4] =
		{
			 { "POSITION", 0, Graphics::Format::RGB_32_Float, 0 }
			,{ "NORMAL", 0, Graphics::Format::RGB_32_Float, 12 }
			,{ "TANGENT", 0,	Graphics::Format::RGB_32_Float, 24 }
			,{ "TEXCOORD", 0, Graphics::Format::RG_32_Float, 36 }
		};

		pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
		pdesc.VertexDescription.Elements = eles;
		pdesc.DepthEnabled = true;
		pdesc.DepthFunction = Graphics::LessEqual;
		pdesc.DepthFormat = Graphics::Depth24_Stencil8;
		pdesc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
		pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		mTestPipeline = mGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}

	// Present pipeline:
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

		mPresentPipeline = mGraphicsInterface->CreateGraphicsPipeline(desc);

		VertexScreen vtxData[6] =
		{
			-1.0f, 1.0f,0.0f,
			1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,

			-1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
		};
		mPresentVtxBuffer = mGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::None, sizeof(VertexScreen) * 6, &vtxData);
	}

	mAppDataCB = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));
}

void TestRenderer::Release()
{
}

void TestRenderer::Render(SceneGraph* scene)
{
	Platform::BaseWindow* outputWindow = mOwnerApp->GetWindow();
	 
	mGraphicsInterface->SetScissor(0, 0, outputWindow->GetWidth(), outputWindow->GetHeight());

	// Render to screen buffer
	mGraphicsInterface->SetTargets(1, &mColourRt, &mDepthRt);
	float clear[4] = { 0.0f,0.0f,0.0f,1.0f };
	mGraphicsInterface->ClearTargets(1, &mColourRt, clear, &mDepthRt, 1.0f, 0);
	{
		mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		mGraphicsInterface->SetGraphicsPipeline(mTestPipeline);
		mAppData.View = glm::lookAt(glm::vec3(0.0f, 0.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mAppData.Projection = glm::perspective(glm::radians(90.0f), 1920.0f / 1080.0f, 0.1f, 200.0f);

		for (uint32_t actorIdx = 0; actorIdx < scene->mRoot->GetNumChilds(); ++actorIdx)
		{
			Renderable* cur = (Renderable*)scene->mRoot->GetChild(actorIdx);
			if (!cur)
			{
				continue;
			}

			mAppData.Model = glm::mat4(1.0f);
			mAppData.Model = glm::translate(mAppData.Model, cur->GetPosition());
			mAppData.Model = glm::rotate(mAppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			mAppData.Model = glm::rotate(mAppData.Model, mOwnerApp->TotalTime * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
			mAppData.Model = glm::rotate(mAppData.Model, mOwnerApp->TotalTime* 2.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			mAppData.Model = glm::scale(mAppData.Model, glm::vec3(1.0f, 1.0f, 1.0f));
			mAppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			
			Mesh* meshes = cur->GetModel()->Meshes;
			mGraphicsInterface->SetConstantBuffer(mAppDataCB, 0, sizeof(AppData), &mAppData);
			mGraphicsInterface->SetVertexBuffer(meshes[0].VertexBuffer, meshes[0].VertexSize * meshes[0].NumVertex, meshes[0].VertexSize);
			mGraphicsInterface->SetIndexBuffer(meshes[0].IndexBuffer, meshes[0].NumIndices * sizeof(uint32_t), Graphics::Format::R_32_Uint);
			mGraphicsInterface->DrawIndexed(meshes[0].NumIndices);
		}
	}
	mGraphicsInterface->DisableAllTargets();

	// Output to the screen
	mGraphicsInterface->SetGraphicsPipeline(mPresentPipeline);
	mGraphicsInterface->SetVertexBuffer(mPresentVtxBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
	mGraphicsInterface->SetResource(mColourRt, 0);
	mGraphicsInterface->Draw(6, 0);
}
