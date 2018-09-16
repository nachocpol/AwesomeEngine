#define NOMINMAX

#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/AssetImporter.h"
#include "Graphics/UI/UIInterface.h"
#include "Graphics/Platform/Windows/WindowsTime.h"
#include "ShowcaseScene.h"

bool InitSystems();
void Resize(int w, int h);

Graphics::Platform::BaseWindow* gWindow = nullptr;
Graphics::GraphicsInterface* gGraphics = nullptr;
Graphics::AssetImporter* gAssetImporter = nullptr;
ShowcaseScene* gScene = nullptr;
Graphics::UI::UIInterface* gUIInterface = nullptr;

Graphics::Platform::Windows::WindowsTimer gTimer;

int main()
{
	InitSystems();
	
	gScene = new ShowcaseScene(gGraphics, gAssetImporter);
	gScene->Initialize();

	Resize(1280, 920);
	gGraphics->FlushAndWait();

	float curDeltaMs = 16.0f;
	bool running = true;
	while (running)
	{
		gTimer.Start();
		gGraphics->StartFrame();

		gUIInterface->StartFrame();
		gWindow->Update();
		gScene->Update(curDeltaMs);

		gScene->Draw(curDeltaMs);
		
		gUIInterface->EndFrame();
		gGraphics->EndFrame();

		running = !gWindow->IsClosed();

		curDeltaMs = gTimer.Stop();
	}

	return 1;
}

bool InitSystems()
{
	gWindow = new Graphics::Platform::Windows::WWindow();
	gWindow->Initialize("Awesome Showcase", false, 1280, 920);

	gGraphics = new Graphics::DX12::DX12GraphicsInterface();
	gGraphics->Initialize(gWindow);

	gAssetImporter = new Graphics::AssetImporter(gGraphics);

	gUIInterface = new Graphics::UI::UIInterface;
	gUIInterface->Initialize(gWindow,gGraphics);

	return true;
}

void Resize(int w, int h)
{
	gScene->Resize(w, h);
}