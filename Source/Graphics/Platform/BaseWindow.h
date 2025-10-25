#pragma once

namespace Graphics { namespace Platform {

	enum class WindowMode
	{
		Windowed,
		Borderless,
		Fullscreen	
	};

	struct WindowInitParams
	{
		const char* m_Title;
		WindowMode m_Mode;
	};

	class BaseWindow
	{
	public:
		BaseWindow();
		~BaseWindow();
		virtual bool Initialize(const WindowInitParams& params) = 0;
		bool IsFullScreen();
		virtual void SetFullScreen(bool newFs) = 0;
		virtual void Update() = 0;
		bool IsClosed();
		virtual void* GetHandle() = 0;
		unsigned int GetWidth();
		unsigned int GetHeight();

	protected:
		bool m_Closed;
		WindowInitParams m_InitParams;
		int m_Width;
		int m_Height;
	};
}}