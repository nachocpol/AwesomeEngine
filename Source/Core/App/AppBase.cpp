#include "AppBase.h"

#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/DX12/DX12GraphicsInterface.h"

AppBase::AppBase():
	 m_Name("Defaul")
	,m_FullScreen(false)
{
}

AppBase::~AppBase()
{
}

void AppBase::Configure(const char* name, bool fullScreen)
{
	m_Name = name;
	m_FullScreen = fullScreen;
}

void AppBase::Init()
{
	m_Window = new Graphics::Platform::Windows::WWindow();
	m_Window->Initialize(m_Name, m_FullScreen, 720, 360);

	m_GraphicsInterface = new Graphics::DX12::DX12GraphicsInterface();
	m_GraphicsInterface->Initialize(m_Window);
}

void AppBase::Update()
{
}

void AppBase::Release()
{
}

bool AppBase::Running()
{
	return !m_Window->IsClosed();
}
