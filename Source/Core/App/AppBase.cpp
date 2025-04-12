#include "AppBase.h"

#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/UI/UIInterface.h"
#include "Graphics/DebugDraw.h"
#include "Core/FileSystem.h"
#include "Core/Logging.h"

AppBase::AppBase() 
	: TotalTime(0.0f)
	, DeltaTime(0.0f)
	, m_GraphicsInterface(nullptr)
	, m_UIInterface(nullptr)
	, m_Window(nullptr)
	, m_Name("Default")
	, m_FullScreen(false)
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
	Core::FileSystem::GetInstance()->Initialize();

	m_Window = new Graphics::Platform::Windows::WWindow();
	m_Window->Initialize(m_Name, m_FullScreen, 1920, 1080);

	m_GraphicsInterface = new Graphics::DX12::DX12GraphicsInterface();
	m_GraphicsInterface->Initialize(m_Window);

	m_UIInterface = new Graphics::UI::UIInterface();
	m_UIInterface->Initialize(m_Window, m_GraphicsInterface);

	Graphics::DebugDraw::GetInstance()->Initialize(m_GraphicsInterface);
}

void AppBase::StartFrame()
{
	m_Window->Update(); // This will query new events...
	m_GraphicsInterface->StartFrame();
	m_UIInterface->StartFrame();
	Graphics::DebugDraw::GetInstance()->StartFrame();
}

void AppBase::Update()
{
}

void AppBase::EndFrame()
{
	Logger::GetInstance()->Render();
	Graphics::DebugDraw::GetInstance()->EndFrame();
	m_UIInterface->EndFrame();
	m_GraphicsInterface->EndFrame();
}

void AppBase::Release()
{
}

bool AppBase::Running()
{
	return !m_Window->IsClosed();
}

Graphics::GraphicsInterface* AppBase::GetGraphicsInterface() const
{
	return m_GraphicsInterface;
}

Graphics::Platform::BaseWindow* AppBase::GetWindow() const
{
	return m_Window;
}
