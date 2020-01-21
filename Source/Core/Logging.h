#pragma once

#include <stdio.h>
#include <windows.h> // this is not cross plat.

namespace Core
{

}

#define OUT_DEBUG_STR(str) OutputDebugStringA(str);


static void OUT_IMPL(const char* msg, const char* type, const char* file, int line, ...)
{
	char buff[1024];
	va_list argptr;
	va_start(argptr, line);
	vsnprintf_s(buff, 1024, msg, argptr);

	// Make short version of file:
	char fileName[128];
	char fileExt[8];
	_splitpath_s(file, NULL, 0, NULL, 0, fileName, 128, fileExt, 8);

	char fullMsg[2048];
	sprintf(fullMsg, "%s(%s%s)(%d) %s \n", type, fileName, fileExt, line, buff);

	OUT_DEBUG_STR(fullMsg);
	va_end(argptr);
}

#define INFO(msg, ...) OUT_IMPL(msg, "[INFORMATION]", __FILE__, __LINE__,  __VA_ARGS__)
#define WARN(msg, ...) OUT_IMPL(msg, "[WARNING]", __FILE__, __LINE__,  __VA_ARGS__)
#define ERR(msg, ...)  OUT_IMPL(msg, "[ERROR]", __FILE__, __LINE__,  __VA_ARGS__)