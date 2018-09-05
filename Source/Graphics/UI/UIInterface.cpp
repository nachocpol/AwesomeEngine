#include "UIInterface.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/Platform/BaseWindow.h"
#include "Graphics/Platform/InputManager.h"
#include "IMGUI/imgui.h"

namespace Graphics{namespace UI{

	UIInterface::UIInterface():
		mGraphicsInterface(nullptr),
		mVertexBuffer(InvalidBuffer),
		mIndexBuffer(InvalidBuffer),
		mGraphicsPipeline(InvalidGraphicsPipeline)
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
		ImGui::Render();

		// Process the ImGUI cmd lists
		ImDrawData* drawPipe = ImGui::GetDrawData();
		if (drawPipe)
		{
			for (int i = 0; i < drawPipe->CmdListsCount; i++)
			{
				const ImDrawList* cmdList = drawPipe->CmdLists[i];
				cmdList->VtxBuffer;
				cmdList->IdxBuffer;
				
				for (int j = 0; j < cmdList->CmdBuffer.size(); j++)
				{
					const ImDrawCmd* curCmd = &cmdList->CmdBuffer[j];
					if(curCmd->UserCallback)
					{
						curCmd->UserCallback(cmdList, curCmd);
					}
					else
					{
						std::cout << curCmd->ElemCount << std::endl;
					}
				}
			}
		}
	}

	void UIInterface::Release()
	{
	}

	void UIInterface::CreateUIResources()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData = 0;
		int w = 0;
		int h = 0;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &w, &h);
		Graphics::TextureHandle fontTex;
		fontTex = mGraphicsInterface->CreateTexture2D(w, h, 1, 1, Format::RGBA_8_Unorm, TextureFlags::TextureFlagNone, fontData);
		io.Fonts->SetTexID((void*)fontTex.Handle);
	}

}}
