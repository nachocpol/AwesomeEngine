#include "BaseWindow.h"

namespace Graphics {namespace Platform {

	BaseWindow::BaseWindow():
		mFullScreen(false),
		mWidth(0),
		mHeight(0),
		mTitle("empty"),
		mClosed(true)
	{
	}

	BaseWindow::~BaseWindow()
	{
	}

	bool BaseWindow::IsFullScreen()
	{
		return mFullScreen;
	}

	bool BaseWindow::IsClosed()
	{
		return mClosed;
	}

	unsigned int BaseWindow::GetWidth()
	{
		return mWidth;
	}

	unsigned int BaseWindow::GetHeight()
	{
		return mHeight;
	}
}}