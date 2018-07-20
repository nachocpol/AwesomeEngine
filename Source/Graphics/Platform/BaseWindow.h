#pragma once

namespace Graphics { namespace Platform {

	class BaseWindow
	{
	public:
		BaseWindow();
		~BaseWindow();
		virtual bool Initialize(const char* title,bool fullScreen, unsigned int width, unsigned int height) = 0;
		bool IsFullScreen();
		virtual void SetFullScreen(bool newFs) = 0;
		virtual void Update() = 0;
		bool IsClosed();
		virtual void* GetHandle() = 0;
		unsigned int GetWidth();
		unsigned int GetHeight();

	protected:
		bool mFullScreen;
		bool mClosed;
		unsigned int mWidth;
		unsigned int mHeight;
		const char* mTitle;
	};

}}