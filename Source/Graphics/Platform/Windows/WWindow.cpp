#include "Graphics/Platform/Windows/WWindow.h"
#include <iostream>
#include "Core/Platform/InputManager.h"

using namespace Graphics;
using namespace Graphics::Platform;
using namespace Windows;
using namespace Core;

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	auto inputManager = InputManager::GetInstance();
	switch (msg)
	{
	// KEY UP
	case WM_KEYUP:
		switch (wParam)
		{
			case '1':		inputManager->KeyEvent(Core::KeyType::Num1, KeyState::Released);	break;
			case '2':		inputManager->KeyEvent(Core::KeyType::Num2, KeyState::Released);	break;
			case '3':		inputManager->KeyEvent(Core::KeyType::Num3, KeyState::Released);	break;
			case '4':		inputManager->KeyEvent(Core::KeyType::Num4, KeyState::Released);	break;
			case '5':		inputManager->KeyEvent(Core::KeyType::Num5, KeyState::Released);	break;
			case '6':		inputManager->KeyEvent(Core::KeyType::Num6, KeyState::Released);	break;
			case '7':		inputManager->KeyEvent(Core::KeyType::Num7, KeyState::Released);	break;
			case '8':		inputManager->KeyEvent(Core::KeyType::Num8, KeyState::Released);	break;
			case '9':		inputManager->KeyEvent(Core::KeyType::Num9, KeyState::Released);	break;
			case '0':		inputManager->KeyEvent(Core::KeyType::Num0, KeyState::Released);	break;

			case 'A':		inputManager->KeyEvent(Core::KeyType::A, KeyState::Released);		break;
			case 'B':		inputManager->KeyEvent(Core::KeyType::B, KeyState::Released);		break;
			case 'C':		inputManager->KeyEvent(Core::KeyType::C, KeyState::Released);		break;
			case 'D':		inputManager->KeyEvent(Core::KeyType::D, KeyState::Released);		break;
			case 'E':		inputManager->KeyEvent(Core::KeyType::E, KeyState::Released);		break;
			case 'F':		inputManager->KeyEvent(Core::KeyType::F, KeyState::Released);		break;
			case 'G':		inputManager->KeyEvent(Core::KeyType::G, KeyState::Released);		break;
			case 'H':		inputManager->KeyEvent(Core::KeyType::H, KeyState::Released);		break;
			case 'I':		inputManager->KeyEvent(Core::KeyType::I, KeyState::Released);		break;
			case 'J':		inputManager->KeyEvent(Core::KeyType::J, KeyState::Released);		break;
			case 'K':		inputManager->KeyEvent(Core::KeyType::K, KeyState::Released);		break;
			case 'L':		inputManager->KeyEvent(Core::KeyType::L, KeyState::Released);		break;
			case 'M':		inputManager->KeyEvent(Core::KeyType::M, KeyState::Released);		break;
			case 'N':		inputManager->KeyEvent(Core::KeyType::N, KeyState::Released);		break;
			case 'O':		inputManager->KeyEvent(Core::KeyType::O, KeyState::Released);		break;
			case 'P':		inputManager->KeyEvent(Core::KeyType::P, KeyState::Released);		break;
			case 'Q':		inputManager->KeyEvent(Core::KeyType::Q, KeyState::Released);		break;
			case 'R':		inputManager->KeyEvent(Core::KeyType::R, KeyState::Released);		break;
			case 'S':		inputManager->KeyEvent(Core::KeyType::S, KeyState::Released);		break;
			case 'T':		inputManager->KeyEvent(Core::KeyType::T, KeyState::Released);		break;
			case 'U':		inputManager->KeyEvent(Core::KeyType::U, KeyState::Released);		break;
			case 'V':		inputManager->KeyEvent(Core::KeyType::V, KeyState::Released);		break;
			case 'W':		inputManager->KeyEvent(Core::KeyType::W, KeyState::Released);		break;
			case 'X':		inputManager->KeyEvent(Core::KeyType::X, KeyState::Released);		break;
			case 'Y':		inputManager->KeyEvent(Core::KeyType::Y, KeyState::Released);		break;
			case 'Z':		inputManager->KeyEvent(Core::KeyType::Z, KeyState::Released);		break;

			case VK_ESCAPE:	inputManager->KeyEvent(Core::KeyType::Escape, KeyState::Released);	break;
			case VK_TAB:	inputManager->KeyEvent(Core::KeyType::Tab, KeyState::Released);		break;
			case VK_SPACE:	inputManager->KeyEvent(Core::KeyType::Space, KeyState::Released);	break;
			case VK_RETURN: inputManager->KeyEvent(Core::KeyType::Return, KeyState::Released);	break;
			case VK_CONTROL: inputManager->KeyEvent(Core::KeyType::Control, KeyState::Released);	break;

			default: break;
		}
		break;

	// KEY DOWN
	case WM_KEYDOWN:
		switch (wParam)
		{
			case '1': inputManager->KeyEvent(Core::KeyType::Num1, KeyState::Pressed);			break;
			case '2': inputManager->KeyEvent(Core::KeyType::Num2, KeyState::Pressed);			break;
			case '3': inputManager->KeyEvent(Core::KeyType::Num3, KeyState::Pressed);			break;
			case '4': inputManager->KeyEvent(Core::KeyType::Num4, KeyState::Pressed);			break;
			case '5': inputManager->KeyEvent(Core::KeyType::Num5, KeyState::Pressed);			break;
			case '6': inputManager->KeyEvent(Core::KeyType::Num6, KeyState::Pressed);			break;
			case '7': inputManager->KeyEvent(Core::KeyType::Num7, KeyState::Pressed);			break;
			case '8': inputManager->KeyEvent(Core::KeyType::Num8, KeyState::Pressed);			break;
			case '9': inputManager->KeyEvent(Core::KeyType::Num9, KeyState::Pressed);			break;
			case '0': inputManager->KeyEvent(Core::KeyType::Num0, KeyState::Pressed);			break;

			case 'A':		inputManager->KeyEvent(Core::KeyType::A, KeyState::Pressed);		break;
			case 'B':		inputManager->KeyEvent(Core::KeyType::B, KeyState::Pressed);		break;
			case 'C':		inputManager->KeyEvent(Core::KeyType::C, KeyState::Pressed);		break;
			case 'D':		inputManager->KeyEvent(Core::KeyType::D, KeyState::Pressed);		break;
			case 'E':		inputManager->KeyEvent(Core::KeyType::E, KeyState::Pressed);		break;
			case 'F':		inputManager->KeyEvent(Core::KeyType::F, KeyState::Pressed);		break;
			case 'G':		inputManager->KeyEvent(Core::KeyType::G, KeyState::Pressed);		break;
			case 'H':		inputManager->KeyEvent(Core::KeyType::H, KeyState::Pressed);		break;
			case 'I':		inputManager->KeyEvent(Core::KeyType::I, KeyState::Pressed);		break;
			case 'J':		inputManager->KeyEvent(Core::KeyType::J, KeyState::Pressed);		break;
			case 'K':		inputManager->KeyEvent(Core::KeyType::K, KeyState::Pressed);		break;
			case 'L':		inputManager->KeyEvent(Core::KeyType::L, KeyState::Pressed);		break;
			case 'M':		inputManager->KeyEvent(Core::KeyType::M, KeyState::Pressed);		break;
			case 'N':		inputManager->KeyEvent(Core::KeyType::N, KeyState::Pressed);		break;
			case 'O':		inputManager->KeyEvent(Core::KeyType::O, KeyState::Pressed);		break;
			case 'P':		inputManager->KeyEvent(Core::KeyType::P, KeyState::Pressed);		break;
			case 'Q':		inputManager->KeyEvent(Core::KeyType::Q, KeyState::Pressed);		break;
			case 'R':		inputManager->KeyEvent(Core::KeyType::R, KeyState::Pressed);		break;
			case 'S':		inputManager->KeyEvent(Core::KeyType::S, KeyState::Pressed);		break;
			case 'T':		inputManager->KeyEvent(Core::KeyType::T, KeyState::Pressed);		break;
			case 'U':		inputManager->KeyEvent(Core::KeyType::U, KeyState::Pressed);		break;
			case 'V':		inputManager->KeyEvent(Core::KeyType::V, KeyState::Pressed);		break;
			case 'W':		inputManager->KeyEvent(Core::KeyType::W, KeyState::Pressed);		break;
			case 'X':		inputManager->KeyEvent(Core::KeyType::X, KeyState::Pressed);		break;
			case 'Y':		inputManager->KeyEvent(Core::KeyType::Y, KeyState::Pressed);		break;
			case 'Z':		inputManager->KeyEvent(Core::KeyType::Z, KeyState::Pressed);		break;

			case VK_ESCAPE:	inputManager->KeyEvent(Core::KeyType::Escape, KeyState::Pressed);	break;
			case VK_TAB:	inputManager->KeyEvent(Core::KeyType::Tab, KeyState::Pressed);		break;
			case VK_SPACE:	inputManager->KeyEvent(Core::KeyType::Space, KeyState::Pressed);	break;
			case VK_RETURN: inputManager->KeyEvent(Core::KeyType::Return, KeyState::Pressed);	break;
			case VK_CONTROL: inputManager->KeyEvent(Core::KeyType::Control, KeyState::Pressed);	break;

			default: break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// Left mouse button:
	case WM_LBUTTONDOWN:
		inputManager->MouseButtonEvent(Core::MouseButton::Left, true);
		break;
	case WM_LBUTTONUP:
		inputManager->MouseButtonEvent(Core::MouseButton::Left, false);
		break;

		// Right mouse button:
	case WM_RBUTTONDOWN:
		inputManager->MouseButtonEvent(Core::MouseButton::Right, true);
		break;
	case WM_RBUTTONUP:
		inputManager->MouseButtonEvent(Core::MouseButton::Right, false);
		break;

		// Middle mouse button:
	case WM_MBUTTONDOWN:
		inputManager->MouseButtonEvent(Core::MouseButton::Middle, true);
		break;
	case WM_MBUTTONUP:
		inputManager->MouseButtonEvent(Core::MouseButton::Middle, false);
		break;

	case WM_CHAR:
		if (::IsWindowUnicode(hWnd))
		{
			if (wParam > 0 && wParam < 0x10000)
			{
				inputManager->InputCharEvent((unsigned short)wParam);
			}
		}
		else
		{
			assert(false);
		}
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

bool WWindow::Initialize(const WindowInitParams& params)
{
	m_InitParams = params;

	HMODULE hinst = GetModuleHandle(NULL);

	std::string ctitle(m_InitParams.m_Title);
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

	// Main monitor size in pixels
	int sizeX = GetSystemMetrics(SM_CXSCREEN);
	int sizeY = GetSystemMetrics(SM_CYSCREEN);

	// Right now, we are just making it to the full size of the monitor, note we may be ignoring DPI scale
	m_Width = sizeX;
	m_Height = sizeY;

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
		WS_BORDER,
		CW_USEDEFAULT, CW_USEDEFAULT,
		sizeX, sizeY,
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

	// https://stackoverflow.com/questions/7442939/opening-a-window-that-has-no-title-bar-with-win32
	// TODO: really need to improve this, right now, we are basically always forcing a borderless fullscreen window (ignoring dpi)
	SetWindowLong(mHandle, GWL_STYLE, 0);

	// if (fullscreen)
	// {
	// 	SetWindowLong(hwnd, GWL_STYLE, 0);
	// }

	ShowWindow(mHandle, SW_SHOW);
	UpdateWindow(mHandle);
	m_Closed = false;

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
	//memset(inputManager->m_KeyStates, 0, sizeof(inputManager->m_KeyStates));
	//memset(inputManager->SpecialKeyStates, 0, sizeof(inputManager->SpecialKeyStates));

	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			m_Closed = true;
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