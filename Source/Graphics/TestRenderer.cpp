#define NOMINMAX

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
		pdesc.BlendTargets[0].Enabled = false;
		pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

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

	mCameraDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(Declarations::CameraData));
	mItemDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(Declarations::ItemData));

	mLightsListSB = mGraphicsInterface->CreateBuffer(BufferType::GPUBuffer, CPUAccess::None, GPUAccess::Read, kMaxLightsPerDraw, Declarations::kLightsStride);
}

void TestRenderer::Release()
{
}

static bool kFreezeCulling = false;
static bool kRenderBounds = false;
static bool kRenderLightBounds = false;

void TestRenderer::Render(SceneGraph* scene)
{ 
	// Render UI:
	ImGui::Begin("Renderer");
	ImGui::Checkbox("Render Bounds", &kRenderBounds);
	ImGui::Checkbox("Render Light Bounds", &kRenderLightBounds);
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
		auto sceneLights = scene->GetLights();
		mCurLightsData.resize(glm::min(kMaxLightsPerDraw, (int)sceneLights.size()));
		for (uint32_t i = 0; i < mCurLightsData.size(); ++i)
		{
			const auto light = sceneLights[i];
			Declarations::Light dataLight;
			dataLight.Color = light->GetColor();
			dataLight.Type = (int)light->GetLightType();
			dataLight.Radius = light->GetRadius();
			dataLight.Intensity = light->GetIntensity();
			dataLight.PosDirection = light->GetPosition();
			mCurLightsData[i] = dataLight;

			if (kRenderLightBounds)
			{
				DebugDraw::GetInstance()->DrawWireSphere(dataLight.PosDirection, dataLight.Radius, glm::vec4(light->GetColor(), 1.0f));
			}
		}
		mCurLightCount = (int)mCurLightsData.size();
		mGraphicsInterface->SetBufferData(mLightsListSB, Declarations::kLightsStride * mCurLightCount, 0, mCurLightsData.data());

		// Render items:
		RenderItems(camera, renderSet);

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

	DrawTiledCamera(camera);

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

void TestRenderer::RenderItems(World::Camera* camera, const std::vector<RenderItem>& renderSet)
{
	Platform::BaseWindow* outputWindow = mOwnerApp->GetWindow();

	mGraphicsInterface->SetScissor(0, 0, outputWindow->GetWidth(), outputWindow->GetHeight());
	mGraphicsInterface->SetTargets(1, &mColourRt, &mDepthRt);
	float clear[4] = { 0.4f,0.4f,0.6f,0.0f };
	mGraphicsInterface->ClearTargets(1, &mColourRt, clear, &mDepthRt, 1.0f, 0);
	{
		mGraphicsInterface->SetGraphicsPipeline(mSurfacePipelineBase);
		mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		mCameraData.InvViewProj = camera->GetProjection() * camera->GetInvViewTransform();

		for (const RenderItem& item : renderSet)
		{
			mItemData.World = item.WorldMatrix;
			mItemData.NumLights = mCurLightCount;

			Mesh* meshes = item.Meshes;
			mGraphicsInterface->SetConstantBuffer(mCameraDataCb, Declarations::kCameraDataSlot, sizeof(Declarations::CameraData), &mCameraData);
			mGraphicsInterface->SetConstantBuffer(mItemDataCb, Declarations::kItemDataSlot, sizeof(Declarations::ItemData), &mItemData);
			mGraphicsInterface->SetResource(mLightsListSB, Declarations::kLightsSlot);
			mGraphicsInterface->SetVertexBuffer(meshes[0].VertexBuffer, meshes[0].VertexSize * meshes[0].NumVertex, meshes[0].VertexSize);
			mGraphicsInterface->SetIndexBuffer(meshes[0].IndexBuffer, meshes[0].NumIndices * sizeof(uint32_t), Graphics::Format::R_32_Uint);
			mGraphicsInterface->DrawIndexed(meshes[0].NumIndices);
		}
	}
}

void TestRenderer::DrawOriginGizmo()
{
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	DebugDraw::GetInstance()->DrawLine(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
}

void Graphics::TestRenderer::DrawTiledCamera(World::Camera* camera)
{
	Camera::ProjectionProps camProperties = camera->GetProjectionProps();
	glm::mat4 invView = camera->GetInvViewTransform();
	if (mFreezeCullingState.Enabled)
	{
		invView = mFreezeCullingState.InverseView;
	}
	glm::mat4 viewTrans = glm::inverse(invView);

	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	camProperties.Far *= 0.5f;

	// TO-DO: there is some miss match btw this and the camera frustum debug draw...
	// also, seems that having even numbers ends up with one extra tile!!
	int numTilesW = 11;
	int numTilesH = 5;

	// Base properties of the near plane:
	float halfVFOV = glm::radians(camProperties.VFov * 0.5f);
	float nearH = glm::tan(halfVFOV) * camProperties.Near * 2.0f;
	float nearW = camProperties.Aspect * nearH;

	float deltaW = nearW / (float)numTilesW;
	float deltaH = nearH / (float)numTilesH;

	float curW = -(nearW * 0.5f);
	float curH = nearH * 0.5f;

	float farOveNear = camProperties.Far / camProperties.Near;

	for (float x = curW; x < -curW; x += deltaW)
	{
		for (float y = curH; y > -curH; y -= deltaH)
		{
			// Near
			glm::vec3 TLN = glm::vec3(x, y, camProperties.Near);
			glm::vec3 TRN = glm::vec3(x + deltaW, y, camProperties.Near);

			glm::vec3 BLN = glm::vec3(x, y - deltaH, camProperties.Near);
			glm::vec3 BRN = glm::vec3(x + deltaW, y - deltaH, camProperties.Near);

			// Far
			float TLFDist = farOveNear * glm::length(TLN);
			glm::vec3 TLF = TLN + glm::normalize(TLN) * TLFDist;
			float TRFDist = farOveNear * glm::length(TRN);
			glm::vec3 TRF = TRN + glm::normalize(TRN) * TRFDist;

			float BLFDist = farOveNear * glm::length(BLN);
			glm::vec3 BLF = BLN + glm::normalize(BLN) * BLFDist;
			float BRFDist = farOveNear * glm::length(BRN);
			glm::vec3 BRF = BRN + glm::normalize(BRN) * BRFDist;

			TLN = viewTrans * glm::vec4(TLN, 1.0f);
			TRN = viewTrans * glm::vec4(TRN, 1.0f);
			BLN = viewTrans * glm::vec4(BLN, 1.0f);
			BRN = viewTrans * glm::vec4(BRN, 1.0f);

			TLF = viewTrans * glm::vec4(TLF, 1.0f);
			TRF = viewTrans * glm::vec4(TRF, 1.0f);
			BLF = viewTrans * glm::vec4(BLF, 1.0f);
			BRF = viewTrans * glm::vec4(BRF, 1.0f);

			// Near
			DebugDraw::GetInstance()->DrawLine(TLN, TRN, color);
			DebugDraw::GetInstance()->DrawLine(TRN, BRN, color);
			DebugDraw::GetInstance()->DrawLine(BRN, BLN, color);
			DebugDraw::GetInstance()->DrawLine(BLN, TLN, color);

			// Far
			DebugDraw::GetInstance()->DrawLine(TLF, TRF, color);
			DebugDraw::GetInstance()->DrawLine(TRF, BRF, color);
			DebugDraw::GetInstance()->DrawLine(BRF, BLF, color);
			DebugDraw::GetInstance()->DrawLine(BLF, TLF, color);

			// Sides
			DebugDraw::GetInstance()->DrawLine(TLN, TLF, color);
			DebugDraw::GetInstance()->DrawLine(TRN, TRF, color);
			DebugDraw::GetInstance()->DrawLine(BLN, BLF, color);
			DebugDraw::GetInstance()->DrawLine(BRN, BRF, color);
		}
	}
}
