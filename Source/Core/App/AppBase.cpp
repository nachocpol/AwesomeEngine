#include "AppBase.h"

#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/UI/UIInterface.h"

AppBase::AppBase():
	 TotalTime(0.0f)
	,DeltaTime(0.0f)
	,mName("Default")
	,mFullScreen(false)
{
}

AppBase::~AppBase()
{
}

void AppBase::Configure(const char* name, bool fullScreen)
{
	mName = name;
	mFullScreen = fullScreen;
}

void AppBase::Init()
{
	mWindow = new Graphics::Platform::Windows::WWindow();
	mWindow->Initialize(mName, mFullScreen, 1920, 1080);

	mGraphicsInterface = new Graphics::DX12::DX12GraphicsInterface();
	mGraphicsInterface->Initialize(mWindow);

	mUIInterface = new Graphics::UI::UIInterface();
	mUIInterface->Initialize(mWindow, mGraphicsInterface);
}

void AppBase::StartFrame()
{
	mWindow->Update(); // This will query new events...
	mGraphicsInterface->StartFrame();
	mUIInterface->StartFrame();
}

void AppBase::Update()
{
}

void AppBase::EndFrame()
{
	mUIInterface->EndFrame();
	mGraphicsInterface->EndFrame();
}

void AppBase::Release()
{
}

bool AppBase::Running()
{
	return !mWindow->IsClosed();
}

Graphics::GraphicsInterface* AppBase::GetGraphicsInterface() const
{
	return mGraphicsInterface;
}

Graphics::Platform::BaseWindow* AppBase::GetWindow() const
{
	return mWindow;
}
