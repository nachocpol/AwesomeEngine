#pragma once

#include "RendererBase.h"
#include "GraphicsInterface.h"
#include "Core/Math.h"
#include "HLSL/Declarations.h"
#include "World/Model.h"

#include <vector>

namespace World
{
	class Actor;
	class Camera;
	class CameraComponent;
}

namespace Graphics
{
	struct Mesh;
	struct RenderItem
	{
		glm::mat4 WorldMatrix;
		Mesh* MeshItem;
		MaterialInfo Material;
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
		void ProcessVisibility(World::CameraComponent* camera, const std::vector<World::Actor*>& actors, std::vector<RenderItem>& renderItems);
		void PrepareTiledCamera(World::CameraComponent* camera);
		void RenderItems(World::CameraComponent* camera,const std::vector<RenderItem>& renderSet);
		void DrawOriginGizmo();
		void DrawTiledCamera(World::CameraComponent* camera);

		TextureHandle mColourRt;
		TextureHandle mDepthRt;
		GraphicsPipeline mTestPipeline;
		GraphicsPipeline mSurfacePipelineBase;
		GraphicsPipeline mPresentPipeline;
		BufferHandle mPresentVtxBuffer;
		Declarations::CameraData mCameraData;
		BufferHandle mCameraDataCb;
		Declarations::ItemData mItemData;
		BufferHandle mItemDataCb;

		// Envmap processing:
		Declarations::IBLData mIBLData;
		BufferHandle mIBLDataCB;
		ComputePipeline mGenIrradianceMapPipeline;
		ComputePipeline mGenPrefilteredMapPipeline;
		ComputePipeline mGenBRDFLutPipeline;
		TextureHandle mBRDFLut;

		// Lights:
		int mCurLightCount;
		int kMaxLights = 1024; 
		std::vector<Declarations::Light> mCurLightsData;
		BufferHandle mLightsListSB;

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