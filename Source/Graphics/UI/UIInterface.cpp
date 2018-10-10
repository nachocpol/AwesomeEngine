#include "UIInterface.h"
#include "Graphics/Platform/BaseWindow.h"
#include "Graphics/Platform/InputManager.h"
#include "IMGUI/imgui.h"

float gVtxUsage = 0.0f;
float gIdxUsage = 0.0f;

namespace Graphics{namespace UI{

	UIInterface::UIInterface():
		mGraphicsInterface(nullptr),
		mVertexBuffer(InvalidBuffer),
		mIndexBuffer(InvalidBuffer),
		mGraphicsPipeline(InvalidGraphicsPipeline),
		mMaxVertices(0),
		mMaxIndices(0)
	{
	}

	UIInterface::~UIInterface()
	{
	
	}

	bool UIInterface::Initialize(Graphics::Platform::BaseWindow* window, GraphicsInterface* graphicsInterface)
	{
		mOutputWindow = window;
		mGraphicsInterface = graphicsInterface;
	
		ImGui::CreateContext();
		CreateUIResources();
		return true;
	}

	void UIInterface::StartFrame()
	{
		Platform::InputManager* inputManager = Platform::InputManager::GetInstance();

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		io.DisplaySize = ImVec2(mOutputWindow->GetWidth(), mOutputWindow->GetHeight());

		// Setup time step
		io.DeltaTime = 0.16f;

		// Mouse buttons
		io.MouseDown[0] = inputManager->IsMouseButtonPressed(Platform::MouseButton::Left);
		io.MouseDown[1] = inputManager->IsMouseButtonPressed(Platform::MouseButton::Right);
		io.MouseDown[2] = inputManager->IsMouseButtonPressed(Platform::MouseButton::Middle);

		// Read keyboard modifiers inputs
		// io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		// io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
		// io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
		// io.KeySuper = false;
		// io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

		// Update OS mouse position
		// Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
		if (io.WantSetMousePos)
		{
			// POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
			// ::ClientToScreen(mOutputWindow->GetHandle(), &pos);
			// ::SetCursorPos(pos.x, pos.y);
		}

		// Set mouse position
		glm::vec2 mouse = inputManager->GetMousePos();
		io.MousePos = ImVec2(mouse.x, mouse.y);

		// Update OS mouse cursor with the cursor requested by imgui
		//ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
		//if (g_LastMouseCursor != mouse_cursor)
		//{
		//	g_LastMouseCursor = mouse_cursor;
		//	ImGui_ImplWin32_UpdateMouseCursor();
		//}

		ImGui::NewFrame();
	}

	void UIInterface::EndFrame()
	{
		//ImGui::Begin("UI Information");
		//ImGui::Text("Vertex usage:%f%c", gVtxUsage, '%');
		//ImGui::Text("Index usage:%f%c", gIdxUsage, '%');
		//ImGui::End();

		ImGui::Render();

		// Process the ImGUI cmd lists
		ImDrawData* drawPipe = ImGui::GetDrawData();
		if (drawPipe && drawPipe->CmdListsCount > 0)
		{
			UpdateBuffers(drawPipe);
			// Update constant buffer
			{
				float L = drawPipe->DisplayPos.x;
				float R = drawPipe->DisplayPos.x + drawPipe->DisplaySize.x;
				float T = drawPipe->DisplayPos.y;
				float B = drawPipe->DisplayPos.y + drawPipe->DisplaySize.y;
				float tmp[4][4] = 
				{
					{ 2.0f / (R - L),		0.0f,				0.0f,       0.0f },
					{ 0.0f,					2.0f / (T - B),     0.0f,       0.0f },
					{ 0.0f,					0.0f,				0.5f,       0.0f },
					{ (R + L) / (L - R),	(T + B) / (B - T),  0.5f,       1.0f },
				};
				memcpy(&mUIData.matrix, &tmp, sizeof(tmp));
			}

			// Viewport
			mGraphicsInterface->SetViewport(0.0f, 0.0f, drawPipe->DisplaySize.x, drawPipe->DisplaySize.y);

			size_t vtxStride = sizeof(ImDrawVert);
			size_t idxStride = sizeof(ImDrawIdx);
			Graphics::Format idxFmt = idxStride == 2 ? Format::R_16_Uint : Format::R_32_Uint;

			// Bind state
			float imBlend[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float defBlend[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			mGraphicsInterface->SetVertexBuffer(mVertexBuffer, mMaxVertices * vtxStride, vtxStride);
			mGraphicsInterface->SetIndexBuffer(mIndexBuffer, mMaxIndices * idxStride, idxFmt);
			mGraphicsInterface->SetGraphicsPipeline(mGraphicsPipeline);
			mGraphicsInterface->SetConstantBuffer(mUIDataHandle, 0, sizeof(mUIData), &mUIData.matrix);
			mGraphicsInterface->SetBlendFactors(imBlend);
			ImVec2 displayPos = drawPipe->DisplayPos;

			uint32_t idxOffset = 0;
			uint32_t vtxOffset = 0;

			// Iterate over each command list
			for (int i = 0; i < drawPipe->CmdListsCount; i++)
			{
				const ImDrawList* cmdList = drawPipe->CmdLists[i];
				// Iterate over each command
				for (int j = 0; j < cmdList->CmdBuffer.size(); j++)
				{
					const ImDrawCmd* curCmd = &cmdList->CmdBuffer[j];
					if(curCmd->UserCallback)
					{
						curCmd->UserCallback(cmdList, curCmd);
					}
					else
					{
						const Graphics::TextureHandle iTex = { (uint64_t)curCmd->TextureId };
						if (CHECK_TEXTURE(iTex))
						{
							mGraphicsInterface->SetScissor
							(
								curCmd->ClipRect.x - displayPos.x, curCmd->ClipRect.y - displayPos.y, 
								curCmd->ClipRect.z - displayPos.x, curCmd->ClipRect.w - displayPos.y
							);
							mGraphicsInterface->SetResource(iTex,0);
							mGraphicsInterface->DrawIndexed(curCmd->ElemCount, idxOffset, vtxOffset);
						}
						else
						{
							assert(false);
						}
						idxOffset += curCmd->ElemCount;
					}
				}
				vtxOffset += cmdList->VtxBuffer.Size;
			}

			// Careful, we may need to reset the scissor just in case
			// more draw calls come after this!
			mGraphicsInterface->SetBlendFactors(defBlend);
		}
	}

	void UIInterface::Release()
	{
	}

	void UIInterface::CreateUIResources()
	{
		// Font texture
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData = 0;
		int w = 0;
		int h = 0;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &w, &h);
		Graphics::TextureHandle fontTex;
		fontTex = mGraphicsInterface->CreateTexture2D(w, h, 1, 1, Format::RGBA_8_Unorm, TextureFlags::TextureFlagNone, fontData);
		io.Fonts->SetTexID((void*)fontTex.Handle);

		// Buffers
		mMaxVertices = 10000;
		uint64_t vtxBufferSize = sizeof(ImDrawVert) * mMaxVertices;
		mVertexBuffer = mGraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::Write, vtxBufferSize);

		mMaxIndices = 10000;
		uint64_t idxBufferSize = sizeof(ImDrawIdx) * mMaxIndices;
		mIndexBuffer = mGraphicsInterface->CreateBuffer(BufferType::IndexBuffer, CPUAccess::Write, idxBufferSize);

		mUIDataHandle = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, CPUAccess::None, sizeof(mUIData));

		// Render pipeline
		Graphics::GraphicsPipelineDescription pdesc;
		pdesc.VertexShader.ShaderEntryPoint = "VSUI";
		pdesc.VertexShader.ShaderPath = "Common.hlsl";
		pdesc.VertexShader.Type = ShaderType::Vertex;

		pdesc.PixelShader.ShaderEntryPoint = "PSUI";
		pdesc.PixelShader.ShaderPath = "Common.hlsl";
		pdesc.PixelShader.Type = ShaderType::Pixel;

		pdesc.ColorFormats[0] = mGraphicsInterface->GetOutputFormat();
		pdesc.DepthEnabled = false;
		pdesc.DepthWriteEnabled = false;
		pdesc.DepthFunction = Graphics::DepthFunc::Always;
		
		int vec2Size = sizeof(float) * 2;
		Graphics::VertexInputDescription::VertexInputElement vtxEles[3] =
		{
			{ "POSITION",0,Graphics::Format::RG_32_Float,0 },
			{ "TEXCOORD",0,Graphics::Format::RG_32_Float,vec2Size * 1 },
			{ "COLOR",0,Graphics::Format::RGBA_8_Unorm,vec2Size * 2 }
		};
		pdesc.VertexDescription.Elements = vtxEles;
		pdesc.VertexDescription.NumElements = 3;

		pdesc.BlendTargets[0].Enabled = true;

		pdesc.BlendTargets[0].SrcBlendColor = BlendFunction::BlendSrcAlpha;
		pdesc.BlendTargets[0].DstBlendColor = BlendFunction::BlendInvSrcAlpha;
		pdesc.BlendTargets[0].BlendOpColor = BlendOperation::BlendOpAdd;

		pdesc.BlendTargets[0].SrcBlendAlpha = BlendFunction::BlendInvDstAlpha;
		pdesc.BlendTargets[0].DstBlendAlpha = BlendFunction::BlendZero;
		pdesc.BlendTargets[0].BlendOpAlpha = BlendOperation::BlendOpAdd;

		pdesc.BlendTargets[0].WriteMask = 0xF;

		mGraphicsPipeline = mGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}

	void UIInterface::UpdateBuffers(ImDrawData* data)
	{
		size_t vtxStride = sizeof(ImDrawVert);
		size_t idxStride = sizeof(ImDrawIdx);

		uint32_t curVtxOffset = 0;
		uint32_t curIdxOffset = 0;
	
		unsigned char* pVtxData = nullptr;
		unsigned char* pIdxData = nullptr;

		mGraphicsInterface->MapBuffer(mVertexBuffer, &pVtxData);
		mGraphicsInterface->MapBuffer(mIndexBuffer, &pIdxData);

		for (int i = 0; i < data->CmdListsCount; i++)
		{
			ImDrawList* cmdList = data->CmdLists[i];
			uint16_t curVtxSize = cmdList->VtxBuffer.Size * vtxStride;
			uint16_t curIdxSize = cmdList->IdxBuffer.Size * idxStride;

			memcpy(pVtxData + curVtxOffset, cmdList->VtxBuffer.Data, curVtxSize);
			memcpy(pIdxData + curIdxOffset, cmdList->IdxBuffer.Data, curIdxSize);

			curVtxOffset += curVtxSize;
			curIdxOffset += curIdxSize;
		}

		mGraphicsInterface->UnMapBuffer(mVertexBuffer);
		mGraphicsInterface->UnMapBuffer(mIndexBuffer);

		gVtxUsage = ((float)curVtxOffset / float(mMaxVertices * vtxStride)) * 100.0f;
		gIdxUsage = ((float)curIdxOffset / float(mMaxIndices * idxStride)) * 100.0f;
	}

}}
