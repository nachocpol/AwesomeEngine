#pragma once

#include "RendererBase.h"
#include "GraphicsInterface.h"
#include "Core/Math.h"
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

	struct TiledCamera
	{
		struct Tile
		{
			Math::Plane Planes[6];
		};
		TiledCamera() : Tiles(nullptr) {};
		Tile* Tiles; // 2D array
		int Width;
		int Height;
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
		void PrepareTiledCamera(World::Camera* camera);
		void RenderItems(World::Camera* camera,const std::vector<RenderItem>& renderSet);
		void DrawOriginGizmo();
		void DrawTiledCamera(World::Camera* camera);

		Graphics::TextureHandle mColourRt;
		Graphics::TextureHandle mDepthRt;
		Graphics::GraphicsPipeline mTestPipeline;
		Graphics::GraphicsPipeline mSurfacePipelineBase;
		Graphics::GraphicsPipeline mPresentPipeline;
		Graphics::BufferHandle mPresentVtxBuffer;
		Declarations::CameraData mCameraData;
		Graphics::BufferHandle mCameraDataCb;
		Declarations::ItemData mItemData;
		Graphics::BufferHandle mItemDataCb;

		int mCurLightCount;
		int kMaxLights = 1024; 
		std::vector<Declarations::Light> mCurLightsData;
		Graphics::BufferHandle mLightsListSB;

		int kNumTilesW = 11;
		int kNumTilesH = 5;
		int kMaxLightsPerTile = 32;
		TiledCamera mTiledCamera;

		struct CameraState
		{
			bool Enabled;
			glm::mat4 InverseView;
		}mFreezeCullingState; // TO-DO: per camera?
	};
}