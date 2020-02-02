#include "TestRenderer.h"
#include "World/SceneGraph.h"
#include "World/Renderable.h"
#include "World/Model.h"
#include "World/Camera.h"
#include "Core/App/AppBase.h"
#include "Platform/BaseWindow.h"
#include "DebugDraw.h"

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
		pdesc.PixelShader.ShaderEntryPoint = "PSSimple";
		pdesc.PixelShader.ShaderPath = "Common.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSSimple";
		pdesc.VertexShader.ShaderPath = "Common.hlsl";
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
		pdesc.DepthWriteEnabled = true;
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

	mCameraDataCb = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(CameraData));
	mItemDataCb = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(ItemData));
}

void TestRenderer::Release()
{
}

void TestRenderer::Render(SceneGraph* scene)
{ 
	const Actor* rootActor = scene->GetRoot();
	const std::vector<Camera*> cameras = scene->GetCameras();

	// Early exit if nothing needs processing:
	if (cameras.empty() || rootActor == nullptr || rootActor->GetNumChilds() == 0)
	{
		return;
	}

	Platform::BaseWindow* outputWindow = mOwnerApp->GetWindow();

	// For each camera:
	for (Camera* camera : cameras)
	{
		std::vector<RenderItem> renderSet;
		ProcessVisibility(camera, rootActor->GetChilds(), renderSet);

		mGraphicsInterface->SetScissor(0, 0, outputWindow->GetWidth(), outputWindow->GetHeight());

		// Render to screen buffer
		mGraphicsInterface->SetTargets(1, &mColourRt, &mDepthRt);
		float clear[4] = { 0.2f,0.2f,0.3f,1.0f };
		mGraphicsInterface->ClearTargets(1, &mColourRt, clear, &mDepthRt, 1.0f, 0);
		{
			mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
			mGraphicsInterface->SetGraphicsPipeline(mTestPipeline);  
			mCameraData.InvViewProj = camera->GetProjection() * camera->GetInvViewTransform();

			for (const RenderItem& item : renderSet)
			{
				mItemData.World = item.WorldMatrix;

				Mesh* meshes = item.Meshes;
				mGraphicsInterface->SetConstantBuffer(mCameraDataCb, 0, sizeof(CameraData), &mCameraData);
				mGraphicsInterface->SetConstantBuffer(mItemDataCb, 1, sizeof(ItemData), &mItemData);
				mGraphicsInterface->SetVertexBuffer(meshes[0].VertexBuffer, meshes[0].VertexSize * meshes[0].NumVertex, meshes[0].VertexSize);
				mGraphicsInterface->SetIndexBuffer(meshes[0].IndexBuffer, meshes[0].NumIndices * sizeof(uint32_t), Graphics::Format::R_32_Uint);
				mGraphicsInterface->DrawIndexed(meshes[0].NumIndices);
			}
		}

		DrawOriginGizmo();
		
		// Flush debug draw
		DebugDraw::GetInstance()->Flush(camera);

		mGraphicsInterface->DisableAllTargets();

		// Output to the screen
		mGraphicsInterface->SetGraphicsPipeline(mPresentPipeline);
		mGraphicsInterface->SetVertexBuffer(mPresentVtxBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
		mGraphicsInterface->SetResource(mColourRt, 0);
		mGraphicsInterface->Draw(6, 0);
	}
}

void TestRenderer::ProcessVisibility(World::Camera* camera, const std::vector<World::Actor*>& actors, std::vector<RenderItem>& renderItems)
{
	for (Actor* actor : actors)
	{
		// Check if we can render current actor:
		if (actor->GetActorType() == Actor::Type::Renderable)
		{
			Renderable* renderable = (Renderable*)actor;
			RenderItem item;
			
			item.Meshes = renderable->GetModel()->Meshes;
			item.NumMeshes = renderable->GetModel()->NumMeshes;
			item.WorldMatrix = renderable->GetWorldTransform();
			
			const auto aabb = renderable->GetWorldAABB(0);
			DebugDraw::GetInstance()->DrawAABB(aabb.Min, aabb.Max);
			const auto sb = renderable->GetWorldBS(0);
			DebugDraw::GetInstance()->DrawWireSphere(sb.Center, sb.Radius);

			renderItems.push_back(item);
		}
		
		if (actor->GetNumChilds() > 0)
		{
			ProcessVisibility(camera, actor->GetChilds(), renderItems);
		}
	}
}

void TestRenderer::DrawOriginGizmo()
{
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
}
