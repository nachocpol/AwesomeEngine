#pragma once

#include "IMGUI/imgui.h"
#include "Graphics/GraphicsInterface.h"
#include "HLSL/Declarations.h"

namespace Graphics
{ 
	namespace Platform
	{
		class BaseWindow;
	}
	namespace UI
	{
		// This class will process draw lists generated by ImGUI to be processed by 
		// the current graphics API
		class UIInterface
		{
		public:
			UIInterface();
			~UIInterface();
			bool Initialize(Platform::BaseWindow* window, GraphicsInterface* graphicsInterface);
			void StartFrame();
			void EndFrame();
			void Release();
	
		private:
			void CreateUIResources();
			void UpdateBuffers(ImDrawData* data);
			
			GraphicsInterface* mGraphicsInterface;
			Graphics::Platform::BaseWindow* mOutputWindow;
			Graphics::BufferHandle mVertexBuffer;
			Graphics::BufferHandle mIndexBuffer;
			Graphics::GraphicsPipeline mGraphicsPipeline;
			Declarations::UIData mUIData;
			Graphics::BufferHandle mUIDataHandle;

			uint32_t mMaxVertices;
			uint32_t mMaxIndices;
		};
	}
}