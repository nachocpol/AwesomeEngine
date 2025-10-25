#include "BaseWindow.h"

namespace Graphics {namespace Platform {

	BaseWindow::BaseWindow():
		m_Width(0),
		m_Height(0),
		m_Closed(false)
	{
	}

	BaseWindow::~BaseWindow()
	{
	}

	bool BaseWindow::IsFullScreen()
	{
		return false;
	}

	bool BaseWindow::IsClosed()
	{
		return m_Closed;
	}

	unsigned int BaseWindow::GetWidth()
	{
		return m_Width;
	}

	unsigned int BaseWindow::GetHeight()
	{
		return m_Height;
	}
}}