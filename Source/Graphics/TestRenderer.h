#pragma once

#include "RendererBase.h"
#include "GraphicsInterface.h"
#include "HLSL/Declarations.h"

#include <vector>

namespace World
{
	class Actor;
	class Camera;
}

namespace Graphics
{
	struct Mesh;
	struct RenderItem
	{
		glm::mat4 WorldMatrix;
		Mesh* Meshes;// this should be 1 mesh (so rendereables with more that 1 mesh should split)
		uint32_t NumMeshes;
	};
	struct LightItem
	{
		LightData Data;
	};

	class TestRenderer : public RendererBase
	{
	public:
		TestRenderer();
		~TestRenderer();
		void Initialize(AppBase* app);
		void Release();
		void Render(World::SceneGraph* scene)override;

	private:
		void ProcessVisibility(World::Camera* camera, const std::vector<World::Actor*>& actors, std::vector<RenderItem>& renderItems);
		void RenderItems(World::Camera* camera,const std::vector<RenderItem>& renderSet, const std::vector<LightItem>& lights);
		void DrawOriginGizmo();

		Graphics::TextureHandle mColourRt;
		Graphics::TextureHandle mDepthRt;
		Graphics::GraphicsPipeline mTestPipeline;
		Graphics::GraphicsPipeline mSurfacePipelineBase;
		Graphics::GraphicsPipeline mSurfacePipelineBlend;
		Graphics::GraphicsPipeline mPresentPipeline;
		Graphics::BufferHandle mPresentVtxBuffer;
		CameraData mCameraData;
		Graphics::BufferHandle mCameraDataCb;
		ItemData mItemData;
		Graphics::BufferHandle mItemDataCb;
		LightData mLightData;
		Graphics::BufferHandle mLightDataCb;

		struct CameraState
		{
			bool Enabled;
			glm::mat4 InverseView;
		}mFreezeCullingState; // TO-DO: per camera?
	};
}