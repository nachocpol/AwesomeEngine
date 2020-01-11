#include "EntryPoint.h"
#include "App/AppBase.h"

void EntryImpl(AppBase* app)
{
	app->Init();
	while (app->Running())
	{
		app->Update();
	}
	app->Release();
}
