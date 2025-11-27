#include "UIInterface.h"
#include "Graphics/Platform/BaseWindow.h"
#include "Graphics/VertexDescription.h"
#include "Core/Platform/InputManager.h"
#include "IMGUI/imgui.h"

float gVtxUsage = 0.0f;
float gIdxUsage = 0.0f;

#define DEFAULT_DPI_SCALE (1.75f) // TODO: we need to properly retrieve this value

ImGuiKey KeyToImgui(Core::KeyType key)
{
	switch (key)
	{
	case Core::KeyType::Num1:
		return ImGuiKey_1;
	case Core::KeyType::Num2:
		return ImGuiKey_2;
	case Core::KeyType::Num3:
		return ImGuiKey_3;
	case Core::KeyType::Num4:
		return ImGuiKey_4;
	case Core::KeyType::Num5:
		return ImGuiKey_5;
	case Core::KeyType::Num6:
		return ImGuiKey_6;
	case Core::KeyType::Num7:
		return ImGuiKey_7;
	case Core::KeyType::Num8:
		return ImGuiKey_8;
	case Core::KeyType::Num9:
		return ImGuiKey_9;
	case Core::KeyType::Num0:
		return ImGuiKey_0;

	case Core::KeyType::A:
		return ImGuiKey_A;
	case Core::KeyType::B:
		return ImGuiKey_B;
	case Core::KeyType::C:
		return ImGuiKey_C;
	case Core::KeyType::D:
		return ImGuiKey_D;
	case Core::KeyType::E:
		return ImGuiKey_E;
	case Core::KeyType::F:
		return ImGuiKey_F;
	case Core::KeyType::G:
		return ImGuiKey_G;
	case Core::KeyType::H:
		return ImGuiKey_H;
	case Core::KeyType::I:
		return ImGuiKey_I;
	case Core::KeyType::J:
		return ImGuiKey_J;
	case Core::KeyType::K:
		return ImGuiKey_K;
	case Core::KeyType::L:
		return ImGuiKey_L;
	case Core::KeyType::M:
		return ImGuiKey_M;
	case Core::KeyType::N:
		return ImGuiKey_N;
	case Core::KeyType::O:
		return ImGuiKey_O;
	case Core::KeyType::P:
		return ImGuiKey_P;
	case Core::KeyType::Q:
		return ImGuiKey_Q;
	case Core::KeyType::R:
		return ImGuiKey_R;
	case Core::KeyType::S:
		return ImGuiKey_S;
	case Core::KeyType::T:
		return ImGuiKey_T;
	case Core::KeyType::U:
		return ImGuiKey_U;
	case Core::KeyType::V:
		return ImGuiKey_V;
	case Core::KeyType::W:
		return ImGuiKey_W;
	case Core::KeyType::X:
		return ImGuiKey_X;
	case Core::KeyType::Y:
		return ImGuiKey_Y;
	case Core::KeyType::Z:
		return ImGuiKey_Z;

	case Core::KeyType::Escape:
		return ImGuiKey_Escape;
	case Core::KeyType::Space:
		return ImGuiKey_Space;
	case Core::KeyType::Tab:
		return ImGuiKey_Tab;
	case Core::KeyType::Return:
		return ImGuiKey_Enter;
	case Core::KeyType::Control:
		return ImGuiKey_LeftCtrl;

	case Core::KeyType::COUNT:
	default:
		assert(false);
		break;
	}

	return ImGuiKey_Space;
}


ImGuiMouseButton ButtonToImgui(Core::MouseButton btn)
{
	switch (btn)
	{
		case Core::MouseButton::Left:	return ImGuiMouseButton_Left;
		case Core::MouseButton::Right:	return ImGuiMouseButton_Right;
		case Core::MouseButton::Middle: return ImGuiMouseButton_Middle	;
		case Core::MouseButton::COUNT:
		default:
										return ImGuiMouseButton_COUNT;
	}
}

static void ImguiKeyEventCbk(Core::KeyType key, Core::KeyState state)
{
	ImGuiIO& io = ImGui::GetIO();
	const ImGuiKey imguiKey = KeyToImgui(key);
	io.AddKeyEvent(imguiKey, state == Core::KeyState::Pressed);
}

static void ImGuiMouseButtonCbk(Core::MouseButton button, bool pressed)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMouseButtonEvent(ButtonToImgui(button), pressed);
}

static void ImGuiInputCharCbk(unsigned short v)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacterUTF16(v);
}

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

		// Provide a callback for key events
		Core::InputManager* inputManager = Core::InputManager::GetInstance();
		inputManager->SetKeyEventCallback(&ImguiKeyEventCbk);
		inputManager->SetMouseButtonCallback(&ImGuiMouseButtonCbk);
		inputManager->SetInputCharCallback(&ImGuiInputCharCbk);

		// ImGui config
		ImGuiStyle& imguiStyle = ImGui::GetStyle();
		imguiStyle.ScaleAllSizes(DEFAULT_DPI_SCALE);

		// This is likely to be enforced at some point
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags = ImGuiBackendFlags_None;

		CreateUIResources();				

		return true;
	}

	void UIInterface::StartFrame()
	{
		Core::InputManager* inputManager = Core::InputManager::GetInstance();

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		io.DisplaySize = ImVec2((float)mOutputWindow->GetWidth(), (float)mOutputWindow->GetHeight());

		// Setup time step
		io.DeltaTime = 0.01f;

		// Set mouse position
		glm::vec2 mouse = inputManager->GetMousePos();
		io.AddMousePosEvent(mouse.x, mouse.y);

		ImGui::NewFrame();
	}

	void UIInterface::EndFrame()
	{
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
				mUIData.ProjectionUI[0] = glm::vec4(2.0f / (R - L),		0.0f,				0.0f, 0.0f);
				mUIData.ProjectionUI[1] = glm::vec4(0.0f,				2.0f / (T - B),		0.0f, 0.0f);
				mUIData.ProjectionUI[2] = glm::vec4(0.0f,				0.0f,				0.5f, 0.0f);
				mUIData.ProjectionUI[3] = glm::vec4((R + L) / (L - R),	(T + B) / (B - T),	0.5f, 1.0f);
			}

			// Viewport
			mGraphicsInterface->SetViewport(0, 0, (uint32_t)drawPipe->DisplaySize.x, (uint32_t)drawPipe->DisplaySize.y);

			size_t vtxStride = sizeof(ImDrawVert);
			size_t idxStride = sizeof(ImDrawIdx);
			Graphics::Format idxFmt = idxStride == 2 ? Format::R_16_Uint : Format::R_32_Uint;

			// Bind state
			float imBlend[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float defBlend[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			mGraphicsInterface->SetVertexBuffer(mVertexBuffer, mMaxVertices, (int)vtxStride);
			mGraphicsInterface->SetIndexBuffer(mIndexBuffer, mMaxIndices, idxFmt);
			mGraphicsInterface->SetGraphicsPipeline(mGraphicsPipeline);
			mGraphicsInterface->SetBlendFactors(imBlend);
			mGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
			ImVec2 displayPos = drawPipe->DisplayPos;

			int32_t idxOffset = 0;
			int32_t vtxOffset = 0;

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
						mGraphicsInterface->SetConstantBuffer(mUIDataHandle, Declarations::kUIDataSlot, sizeof(mUIData), &mUIData.ProjectionUI);
						curCmd->UserCallback(cmdList, curCmd);
					}
					else
					{
						const Graphics::TextureHandle iTex = { (uint64_t)curCmd->TextureId };
						if (CHECK_TEXTURE(iTex))
						{
							mGraphicsInterface->SetConstantBuffer(mUIDataHandle, Declarations::kUIDataSlot, sizeof(mUIData), &mUIData.ProjectionUI);
							mGraphicsInterface->SetScissor
							(
								(uint32_t)(curCmd->ClipRect.x - displayPos.x), (uint32_t)(curCmd->ClipRect.y - displayPos.y), 
								(uint32_t)(curCmd->ClipRect.z - displayPos.x), (uint32_t)(curCmd->ClipRect.w - displayPos.y)
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
		ImGuiIO& io = ImGui::GetIO();

		// TODO: Here we just setup a default font, doing it here so we can enforce a scale. This is not ideal, I'm assuming a default size and * DPI..
		//       this is likely to break in newer ImGui versions
		ImFontConfig fontConfig = ImFontConfig();
		fontConfig.SizePixels = 15 * DEFAULT_DPI_SCALE;
		io.Fonts->AddFontDefault(&fontConfig);

		unsigned char* fontData = 0;
		int w = 0;
		int h = 0;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &w, &h);
		Graphics::TextureHandle fontTex;
		fontTex = mGraphicsInterface->CreateTexture2D(w, h, 1, 1, Format::RGBA_8_Unorm, TextureFlags::TextureFlagNone, fontData);
		io.Fonts->SetTexID((ImTextureID)fontTex.Handle);

		// Buffers
		mMaxVertices = 50000;
		uint64_t vtxBufferSize = sizeof(ImDrawVert) * mMaxVertices;
		mVertexBuffer = mGraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::Write, GPUAccess::Read, vtxBufferSize, 0, nullptr,"UIVertexBuffer");

		mMaxIndices = 50000;
		uint64_t idxBufferSize = sizeof(ImDrawIdx) * mMaxIndices;
		mIndexBuffer = mGraphicsInterface->CreateBuffer(BufferType::IndexBuffer, CPUAccess::Write, GPUAccess::Read, idxBufferSize, 0, nullptr, "UIIndexBuffer");

		mUIDataHandle = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(mUIData));

		// Render pipeline
		Graphics::GraphicsPipelineDescription pdesc;
		pdesc.VertexShader.ShaderEntryPoint = "VSUI";
		pdesc.VertexShader.ShaderPath = "shadersrc:Common.hlsl";
		pdesc.VertexShader.Type = ShaderType::Vertex;

		pdesc.PixelShader.ShaderEntryPoint = "PSUI";
		pdesc.PixelShader.ShaderPath = "shadersrc:Common.hlsl";
		pdesc.PixelShader.Type = ShaderType::Pixel;

		pdesc.ColorFormats[0] = mGraphicsInterface->GetOutputFormat();
		pdesc.DepthEnabled = false;
		pdesc.DepthWriteEnabled = false;
		pdesc.DepthFunction = Graphics::DepthFunc::Always;
		pdesc.CullMode = FaceCullMode::None;

		pdesc.VertexDescription = Pos2TexCoordColorDescription::GetDescription();

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
		int32_t vtxStride = sizeof(ImDrawVert);
		int32_t idxStride = sizeof(ImDrawIdx);

		int32_t curVtxOffset = 0;
		int32_t curIdxOffset = 0;
	
		unsigned char* pVtxData = nullptr;
		unsigned char* pIdxData = nullptr;

		mGraphicsInterface->MapBuffer(mVertexBuffer, &pVtxData);
		mGraphicsInterface->MapBuffer(mIndexBuffer, &pIdxData);

		for (int i = 0; i < data->CmdListsCount; i++)
		{
			ImDrawList* cmdList = data->CmdLists[i];
			int32_t curVtxSize = (int32_t)cmdList->VtxBuffer.Size * vtxStride;
			int32_t curIdxSize = (int32_t)cmdList->IdxBuffer.Size * idxStride;

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
