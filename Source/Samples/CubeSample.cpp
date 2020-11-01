#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"

class CubeApp : public AppBase
{
	void Init();
	void Update();
	void Release();
};

void CubeApp::Init()
{
	AppBase::Init();
}

void CubeApp::Update()
{
	AppBase::Update();
}

void CubeApp::Release()
{
	AppBase::Release();
}

CubeApp app;
ENTRY_POINT(app, "CubeSample", false);