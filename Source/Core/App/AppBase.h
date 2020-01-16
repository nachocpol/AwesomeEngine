#pragma once

namespace Graphics
{
	namespace Platform
	{
		class BaseWindow;
	}
	namespace UI
	{
		class UIInterface;
	}
	class GraphicsInterface;
}

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();
	void Configure(const char* name, bool fullScreen);
	virtual void Init();
	void StartFrame();
	virtual void Update();
	void EndFrame();
	virtual void Release();
	bool Running();

	// Total time in seconds the app has been running (in s)
	float TotalTime;
	// Last frame delta time (in ms)
	float DeltaTime;

protected:
	Graphics::Platform::BaseWindow* mWindow;
	Graphics::GraphicsInterface* mGraphicsInterface;
	Graphics::UI::UIInterface* mUIInterface;
	const char* mName;
	bool mFullScreen;

private:
	AppBase(const AppBase& other) {};
};