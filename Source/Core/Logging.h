#pragma once

#include <stdio.h>

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

#define INFO(msg, ...) OUT_IMPL("[INFORMATION][%s][%d]:" msg "\n",__FILE__,__LINE__,  __VA_ARGS__);
#define WARN(msg, ...) OUT_IMPL("[WARNING][%s][%d]:" msg "\n",__FILE__,__LINE__,  __VA_ARGS__);
#define ERR(msg, ...)  OUT_IMPL("[ERROR][%s][%d]:" msg "\n",__FILE__,__LINE__,  __VA_ARGS__);