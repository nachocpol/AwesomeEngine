#pragma once

#include <stdio.h>
#include <windows.h> // this is not cross plat.

namespace Core
{

}

#define OUT_DEBUG_STR(str) OutputDebugStringA(str);


static void OUT_IMPL(const char* msg, ...)
{
	char buff[256];
	va_list argptr;
	va_start(argptr, msg);
	vsnprintf_s(buff, 256, msg, argptr);
	OUT_DEBUG_STR(buff);
	va_end(argptr);
}

#define INFO(msg, ...) OUT_IMPL("[INFORMATION][%s][%d]: %s\n",__FILE__,__LINE__,  __VA_ARGS__);
#define WARN(msg, ...) OUT_IMPL("[WARNING][%s][%d]: %s\n",__FILE__,__LINE__,  __VA_ARGS__);
#define ERR(msg, ...)  OUT_IMPL("[ERROR][%s][%d]: %s\n",__FILE__,__LINE__,  __VA_ARGS__);