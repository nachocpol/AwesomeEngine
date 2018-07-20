#include "Graphics/Platform/Windows/WWindow.h"
#include <iostream>

namespace Graphics { namespace Platform {namespace Windows {

	LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) 
			{
				if (MessageBox(0, L"Are you sure you want to exit?",L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					DestroyWindow(hWnd);
				}
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
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
			WS_OVERLAPPEDWINDOW,
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

		return true;
	}

	void WWindow::SetFullScreen(bool newFs)
	{

	}

	void WWindow::Update()
	{
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