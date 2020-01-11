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
	virtual void Update();
	virtual void Release();

	bool Running();

protected:
	Graphics::Platform::BaseWindow* m_Window;
	Graphics::GraphicsInterface* m_GraphicsInterface;
	const char* m_Name;
	bool m_FullScreen;

private:
	AppBase(const AppBase& other) {};
};