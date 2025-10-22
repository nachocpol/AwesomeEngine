#pragma once

namespace Graphics
{
	namespace Platform
	{
		class BaseWindow;
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
	Graphics::GraphicsInterface* GetGraphicsInterface()const;
	Graphics::Platform::BaseWindow* GetWindow()const;
	// Total time in seconds the app has been running (in s)
	float TotalTime;
	// Last frame delta time (in s)
	float DeltaTime;

protected:
	Graphics::Platform::BaseWindow* m_Window;
	Graphics::GraphicsInterface* m_GraphicsInterface;
	const char* m_Name;
	bool m_FullScreen;

private:
	AppBase(const AppBase& other) = default;
};