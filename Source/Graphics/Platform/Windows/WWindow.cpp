#include "Graphics/Platform/Windows/WWindow.h"
#include <iostream>
#include "Graphics/Platform/InputManager.h"

namespace Graphics { namespace Platform {namespace Windows {

	LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		auto inputManager = InputManager::GetInstance();
		switch (msg)
		{
		// KEY UP
		case WM_KEYUP:
			switch (wParam)
			{
			default: inputManager->KeyStates[(unsigned char)wParam] = KeyState::StateUp; printf("UP:%c\n", wParam); break;
			}
			break;

		// KEY DOWN
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_F1:		inputManager->SpecialKeyStates[SpecialKey::F1]	= KeyState::StateDown; break;
				case VK_F2:		inputManager->SpecialKeyStates[SpecialKey::F2]	= KeyState::StateDown; break;
				case VK_F3:		inputManager->SpecialKeyStates[SpecialKey::F3]	= KeyState::StateDown; break;
				case VK_F4:		inputManager->SpecialKeyStates[SpecialKey::F4]	= KeyState::StateDown; break;
				case VK_F5:		inputManager->SpecialKeyStates[SpecialKey::F5]	= KeyState::StateDown; break;
				case VK_F6:		inputManager->SpecialKeyStates[SpecialKey::F6]	= KeyState::StateDown; break;
				case VK_F7:		inputManager->SpecialKeyStates[SpecialKey::F7]	= KeyState::StateDown; break;
				case VK_F8:		inputManager->SpecialKeyStates[SpecialKey::F8]	= KeyState::StateDown; break;
				case VK_F9:		inputManager->SpecialKeyStates[SpecialKey::F9]	= KeyState::StateDown; break;
				case VK_F10:	inputManager->SpecialKeyStates[SpecialKey::F10] = KeyState::StateDown; break;
				case VK_F11:	inputManager->SpecialKeyStates[SpecialKey::F11] = KeyState::StateDown; break;
				case VK_F12:	inputManager->SpecialKeyStates[SpecialKey::F12] = KeyState::StateDown; break;
				case VK_ESCAPE:	inputManager->SpecialKeyStates[SpecialKey::ESC] = KeyState::StateDown; break;
				case VK_TAB:	inputManager->SpecialKeyStates[SpecialKey::TAB]	= KeyState::StateDown; break;
				default:		inputManager->KeyStates[(unsigned char)wParam]			= KeyState::StateDown; printf("DOWN:%c\n", wParam);  break;
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// Left mouse button:
		case WM_LBUTTONDOWN:
			inputManager->MouseButtonStates[MouseButton::Left] = KeyState::StateDown;
			break;
		case WM_LBUTTONUP:
			inputManager->MouseButtonStates[MouseButton::Left] = KeyState::StateUp;
			break;

			// Right mouse button:
		case WM_RBUTTONDOWN:
			inputManager->MouseButtonStates[MouseButton::Right] = KeyState::StateDown;
			break;
		case WM_RBUTTONUP:
			inputManager->MouseButtonStates[MouseButton::Right] = KeyState::StateUp;
			break;

			// Middle mouse button:
		case WM_MBUTTONDOWN:
			inputManager->MouseButtonStates[MouseButton::Middle] = KeyState::StateDown;
			break;
		case WM_MBUTTONUP:
			inputManager->MouseButtonStates[MouseButton::Middle] = KeyState::StateUp;
			break;
		}
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}

	WWindow::WWindow():
		mHandle(0)
	{
	}

	WWindow::~WWindow()
	{
	}

	bool WWindow::Initialize(const char* title, bool fullscreen,unsigned int width, unsigned int height)
	{
		mTitle		= title;
		mFullScreen = fullscreen;
		mWidth		= width;
		mHeight		= height;

		HMODULE hinst = GetModuleHandle(NULL);

		std::string ctitle(mTitle);
		std::wstring wtitle;
		wtitle.assign(ctitle.begin(), ctitle.end());
			
		// if (fullscreen)
		// {
		// 	HMONITOR hmon = MonitorFromWindow(hwnd,
		// 		MONITOR_DEFAULTTONEAREST);
		// 	MONITORINFO mi = { sizeof(mi) };
		// 	GetMonitorInfo(hmon, &mi);
		// 
		// 	width = mi.rcMonitor.right - mi.rcMonitor.left;
		// 	height = mi.rcMonitor.bottom - mi.rcMonitor.top;
		// }

		WNDCLASSEX wndClass = {};
		wndClass.cbSize			= sizeof(WNDCLASSEX);
		wndClass.style			= CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc	= WndProc;
		wndClass.cbClsExtra		= NULL;
		wndClass.cbWndExtra		= NULL;
		wndClass.hInstance		= hinst;
		wndClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 2);
		wndClass.lpszMenuName	= NULL;
		wndClass.lpszClassName	= wtitle.c_str();
		wndClass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wndClass))
		{
			MessageBox(NULL, TEXT("Could not register the window class."), TEXT("Error!"), MB_OK | MB_ICONERROR);
			return false;
		}

		mHandle = CreateWindowEx
		(
			NULL,
			wtitle.c_str(),
			wtitle.c_str(),
			WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			mWidth, mHeight,
			NULL,
			NULL,
			hinst,
			NULL
		);
		if (!mHandle)
		{
			MessageBox(NULL, TEXT("Could not create the window handle."), TEXT("Error!"), MB_OK | MB_ICONERROR);
			return false;
		}

		// if (fullscreen)
		// {
		// 	SetWindowLong(hwnd, GWL_STYLE, 0);
		// }

		ShowWindow(mHandle, SW_SHOW);
		UpdateWindow(mHandle);
		mClosed = false;

		// Provide the input with a handle to this window
		InputManager::GetInstance()->WHandle = mHandle;

		return true;
	}

	void WWindow::SetFullScreen(bool newFs)
	{

	}

	void WWindow::Update()
	{
		auto inputManager = InputManager::GetInstance();
		//memset(inputManager->KeyStates, 0, sizeof(inputManager->KeyStates));
		//memset(inputManager->SpecialKeyStates, 0, sizeof(inputManager->SpecialKeyStates));

		MSG msg = {};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				mClosed = true;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void* WWindow::GetHandle()
	{
		return mHandle;
	}

}}}