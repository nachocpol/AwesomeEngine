#pragma once

#include "Graphics/Platform/BaseWindow.h"
#include <Windows.h>

namespace Graphics { namespace Platform {namespace Windows {

	class WWindow : public BaseWindow
	{
	public:
		WWindow();
		~WWindow();
		bool Initialize(const WindowInitParams& params)final override;
		void SetFullScreen(bool newFs)final override;
		void Update()final override;
		void* GetHandle()final override;

	private:
		HWND mHandle;
	};

}}}