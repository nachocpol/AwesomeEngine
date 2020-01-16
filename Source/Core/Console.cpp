#include "Console.h"

using namespace Core;

Console::Console()
{
}

Console::~Console()
{
}

Console* Core::Console::GetInstance()
{
	static Console* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new Console;
	}
	return kInstance;
}
