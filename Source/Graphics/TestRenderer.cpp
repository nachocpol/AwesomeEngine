#include "TestRenderer.h"
#include "World/SceneGraph.h"
#include "World/Renderable.h"
#include "World/Model.h"
#include "World/Camera.h"
#include "World/Light.h"
#include "Core/App/AppBase.h"
#include "Platform/BaseWindow.h"
#include "DebugDraw.h"
#include "UI/IMGUI/imgui.h"
#include "Core/Logging.h"

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

	// Surface pipeline
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSSurface";
		pdesc.PixelShader.ShaderPath = "Surface.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSSurface";
		pdesc.VertexShader.ShaderPath = "Surface.hlsl";
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
		{
			pdesc.BlendTargets[0].Enabled = true;
			pdesc.BlendTargets[0].BlendOpColor = BlendOperation::BlendOpAdd;
			pdesc.BlendTargets[0].SrcBlendColor = BlendFunction::BlendOne;
			pdesc.BlendTargets[0].DstBlendColor = BlendFunction::BlendOne;

			pdesc.BlendTargets[0].BlendOpAlpha = BlendOperation::BlendOpAdd;
			pdesc.BlendTargets[0].SrcBlendAlpha = BlendFunction::BlendZero;
			pdesc.BlendTargets[0].DstBlendAlpha = BlendFunction::BlendZero;
			pdesc.BlendTargets[0].WriteMask = 14;
		}
		pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		mSurfacePipelineBlend = mGraphicsInterface->CreateGraphicsPipeline(pdesc);

		pdesc.BlendTargets[0].Enabled = false;
		mSurfacePipelineBase = mGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}

	// Present pipeline:
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.DepthEnabled = false;
		desc.DepthFunction = Graphics::Always;
		desc.VertexShader.ShaderEntryPoint = "VSFullScreen";
		desc.VertexShader.ShaderPath = "Common.hlsl";
		desc.VertexShader.Type = Graphics::ShaderType::Vertex;
		desc.PixelShader.ShaderEntryPoint = "PSToneGamma";
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
		mPresentVtxBuffer = mGraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::None, GPUAccess::Read, sizeof(VertexScreen) * 6, 0, &vtxData);
	}

	mCameraDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(CameraData));
	mItemDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(ItemData));
	mLightDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(LightData));

	// Test buffer
	struct TestStruct
	{
		TestStruct()
		{
			Type = 42.0f;
			Pos = glm::vec3(1.0f, 0.0f, 1.0f);
		}
		float Type;
		glm::vec3 Pos;
	};
	TestStruct paco[5];
	auto bu = mGraphicsInterface->CreateBuffer(BufferType::GPUBuffer, CPUAccess::None, GPUAccess::ReadWrite, 5, sizeof(TestStruct), paco);
}

void TestRenderer::Release()
{
}

static bool kFreezeCulling = false;
static bool kRenderBounds = false;

void TestRenderer::Render(SceneGraph* scene)
{ 
	// Render UI:
	ImGui::Begin("Renderer");
	ImGui::Checkbox("Render Bounds", &kRenderBounds);
	ImGui::Checkbox("Freeze Culling", &kFreezeCulling);
	ImGui::End();

	const Actor* rootActor = scene->GetRoot();
	const std::vector<Camera*> cameras = scene->GetCameras();

	// Early exit if nothing needs processing:
	if (cameras.empty() || rootActor == nullptr || rootActor->GetNumChilds() == 0)
	{
		return;
	}

	// For each camera:
	for (Camera* camera : cameras)
	{
		// Cache culling:
		if (kFreezeCulling)
		{
			if (!mFreezeCullingState.Enabled)
			{
				mFreezeCullingState.InverseView = camera->GetInvViewTransform();
			}
			mFreezeCullingState.Enabled = true;
		}
		else
		{
			mFreezeCullingState.Enabled = false;
		}

		// Compute visible items for this camera:
		std::vector<RenderItem> renderSet;
		ProcessVisibility(camera, rootActor->GetChilds(), renderSet);

		// Gather lights: TO-DO: check visibility (we could do per object vis)
		std::vector<LightItem> lights;
		for (const auto light : scene->GetLights())
		{
			LightItem lightData;
			lightData.Data.LightColor = light->GetColor();
			lightData.Data.LightIntensity = light->GetIntensity();
			lightData.Data.LightRadius = light->GetRadius();
			lightData.Data.LightType = (float)light->GetLightType();
			lightData.Data.LightPosDirection = light->GetPosition();
			lights.push_back(lightData);
			//DebugDraw::GetInstance()->DrawWireSphere(
			//	lightData.Data.LightPosDirection, lightData.Data.LightRadius, 
			//	glm::vec4(light->GetColor(), 1.0f)
			//);
		}

		// Render items:
		RenderItems(camera, renderSet, lights);

		DrawOriginGizmo();
		
		// Flush debug draw
		DebugDraw::GetInstance()->Flush(camera);

		mGraphicsInterface->DisableAllTargets();

		// Output to the screen:
		mGraphicsInterface->SetGraphicsPipeline(mPresentPipeline);
		mGraphicsInterface->SetVertexBuffer(mPresentVtxBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
		mGraphicsInterface->SetResource(mColourRt, 0);
		mGraphicsInterface->Draw(6, 0);
	}
}

void TestRenderer::ProcessVisibility(World::Camera* camera, const std::vector<World::Actor*>& actors, std::vector<RenderItem>& renderItems)
{
	const auto projProps = camera->GetProjectionProps();
	glm::mat4 curInvView = mFreezeCullingState.Enabled ? mFreezeCullingState.InverseView : camera->GetInvViewTransform();

	// Draw camera frustum:
	if (mFreezeCullingState.Enabled)
	{
		glm::mat4 viewToWorld = glm::inverse(mFreezeCullingState.InverseView);
		DebugDraw::GetInstance()->DrawFrustum(viewToWorld, projProps.Aspect, projProps.VFov, projProps.Near, projProps.Far, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	}

	// Camera frustum planes in viewspace. TO-DO: this could be computed just once
	Math::Plane cameraFrustumPlanes[6];
	Math::ExtractPlanesFromProjection(cameraFrustumPlanes, projProps.Aspect, projProps.VFov, projProps.Near, projProps.Far);

	// Check for each actor if it is inside the view frustum:
	for (Actor* actor : actors)
	{
		// Check if we can render current actor:
		if (actor->GetActorType() == Actor::Type::Renderable)
		{
			Renderable* renderable = (Renderable*)actor;
			RenderItem item;
			
			const auto aabb = renderable->GetWorldAABB(0);
			const auto sb = renderable->GetWorldBS(0);
			
			Math::BSData bsViewSpace;
			bsViewSpace.Center = curInvView * glm::vec4(sb.Center, 1.0f);
			bsViewSpace.Radius = sb.Radius;
			
			bool inside = true;
			for (uint32_t i = 0; i < 6; ++i)
			{
				auto res = Math::PlaneSphereIntersection(cameraFrustumPlanes[i], bsViewSpace);
				bool curInside = (res == Math::IntersectionResult::Inside) || (res == Math::IntersectionResult::Touching);
				inside &= curInside;
				if (!inside)
				{
					break;
				}
			}

			// Mesh is visible, so add it to the render items:
			if (inside)
			{
				item.Meshes = renderable->GetModel()->Meshes;
				item.NumMeshes = renderable->GetModel()->NumMeshes;
				item.WorldMatrix = renderable->GetWorldTransform();
				
				renderItems.push_back(item);

				// Draw actor bounds:
				if (kRenderBounds)
				{
					DebugDraw::GetInstance()->DrawAABB(aabb.Min, aabb.Max);
					DebugDraw::GetInstance()->DrawWireSphere(sb.Center, sb.Radius);
				}
			}
		}
		
		if (actor->GetNumChilds() > 0)
		{
			ProcessVisibility(camera, actor->GetChilds(), renderItems);
		}
	}
}

void TestRenderer::RenderItems(World::Camera* camera, const std::vector<RenderItem>& renderSet, const std::vector<LightItem>& lights)
{
	Platform::BaseWindow* outputWindow = mOwnerApp->GetWindow();

	mGraphicsInterface->SetScissor(0, 0, outputWindow->GetWidth(), outputWindow->GetHeight());
	mGraphicsInterface->SetTargets(1, &mColourRt, &mDepthRt);
	float clear[4] = { 0.4f,0.4f,0.6f,0.0f };
	mGraphicsInterface->ClearTargets(1, &mColourRt, clear, &mDepthRt, 1.0f, 0);
	{
		mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		mCameraData.InvViewProj = camera->GetProjection() * camera->GetInvViewTransform();
		bool basePass = true;
		for (const LightItem& light : lights)
		{
			mGraphicsInterface->SetGraphicsPipeline(basePass ? mSurfacePipelineBase : mSurfacePipelineBlend);
			basePass = false;
			mLightData = light.Data;
			for (const RenderItem& item : renderSet)
			{
				mItemData.World = item.WorldMatrix;

				Mesh* meshes = item.Meshes;
				mGraphicsInterface->SetConstantBuffer(mCameraDataCb, kCameraDataSlot, sizeof(CameraData), &mCameraData);
				mGraphicsInterface->SetConstantBuffer(mItemDataCb, kItemDataSlot, sizeof(ItemData), &mItemData);
				mGraphicsInterface->SetConstantBuffer(mLightDataCb, kLightDataSlot, sizeof(LightData), &mLightData);
				mGraphicsInterface->SetVertexBuffer(meshes[0].VertexBuffer, meshes[0].VertexSize * meshes[0].NumVertex, meshes[0].VertexSize);
				mGraphicsInterface->SetIndexBuffer(meshes[0].IndexBuffer, meshes[0].NumIndices * sizeof(uint32_t), Graphics::Format::R_32_Uint);
				mGraphicsInterface->DrawIndexed(meshes[0].NumIndices);
			}
		}
	}
}

void TestRenderer::DrawOriginGizmo()
{
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
}
