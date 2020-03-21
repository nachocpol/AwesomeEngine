#pragma once

#include "GraphicsInterface.h"
#include "HLSL/Declarations.h"

#include "glm/glm.hpp"

#include <vector>

namespace World
{
	class CameraComponent;
}

#define MAX_LINES		 4096
#define MAX_WIRE_SPHERES 4096

namespace Graphics
{
	struct Model;
	class GraphicsInterface;
	class DebugDraw
	{
	private:
		DebugDraw();
		DebugDraw(const DebugDraw& other) {};
		~DebugDraw();

	public:
		static DebugDraw* GetInstance();
		void Initialize(GraphicsInterface* graphicsInterface);
		void Release();

		void StartFrame();
		void Flush(World::CameraComponent* camera);
		void EndFrame();
		
		void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0f));
		void DrawAABB(glm::vec3 min, glm::vec3 max, glm::vec4 color = glm::vec4(1.0f));
		void DrawWireSphere(glm::vec3 center, float radius, glm::vec4 color = glm::vec4(1.0f));
		void DrawFrustum(glm::mat4 transform, float aspect, float vfov, float nearDist, float farDist, glm::vec4 color = glm::vec4(1.0f));

		void DrawCubemap(TextureHandle texture, glm::vec3 position, bool equirectangular = false);

	private:
		GraphicsInterface* mGraphicsInterface;
		GraphicsPipeline mDebugPipelineLines;
		GraphicsPipeline mDebugPipelineSolid;

		Declarations::CameraData mCameraData;
		BufferHandle mCameraDataCb;

		Declarations::ItemData mItemData;
		BufferHandle mItemDataCb;

		struct LineItem
		{
			LineItem(glm::vec3 start, glm::vec3 end, glm::vec4 color);
			glm::vec3 Start; // World space
			glm::vec3 End;	 // World space
			glm::vec4 Color;
		};
		std::vector<LineItem> mLines;
		BufferHandle mLinesVtxBuffer;

		struct WireSphereItem
		{
			WireSphereItem(glm::vec3 center, float radius, glm::vec4 color);
			glm::vec3 Center; // World space
			float Radius;
			glm::vec4 Color;
		};
		std::vector<WireSphereItem> mWireSpheres;
		BufferHandle mWireSphereVtxBuffer;
		uint32_t mWireSphereNumVtx;

		struct CubemapItem
		{
			CubemapItem(TextureHandle t, glm::vec3 pos, bool equirect);
			TextureHandle Texture;
			glm::vec3 Position;
			bool Equirectangular;
		};
		std::vector<CubemapItem> mCubemapItems;

		Declarations::DebugData mDebugData;
		BufferHandle mDebugDataCB;
		Model* mSphereModel;
	};
}