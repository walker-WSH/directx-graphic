#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <assert.h>

namespace graphic {

//----------------------------------------------------------------------------------
// in future you should replace it with real LOG
static void TestLogFunc(bool warn, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char buf[4096];
	vsnprintf_s(buf, sizeof(buf) / sizeof(buf[0]), fmt, args);

	va_end(args);

	OutputDebugStringA(buf);
	OutputDebugStringA("\n");

	if (warn)
		OutputDebugStringA("ERROR LOG \n");
}

#define LOG_TRACE() TestLogFunc(false, "--------- %s ---------", __FUNCTION__)
#define LOG_INFO(fmt, ...) TestLogFunc(false, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) TestLogFunc(true, fmt, __VA_ARGS__)

} // namespace graphic
