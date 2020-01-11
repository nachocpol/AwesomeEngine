#pragma once

class AppBase;

void EntryImpl(AppBase* app);

#define ENTRY_POINT(appVar, appName, fullScreen) \
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) \
{ \
appVar.Configure(appName, fullScreen); \
EntryImpl(&appVar); \
} \

