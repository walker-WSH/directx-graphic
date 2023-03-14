#pragma once
#include <string>
#include <TCHAR.H>
#include <Windows.h>
#include <shlwapi.h>

namespace winHelper {

std::wstring GetExeFullPath();
std::wstring GetFileDirectory(HINSTANCE hInstance = NULL);

std::string ExtractFileName(const char *full_path);
std::string ExtractExtension(const char *full_path);

bool IsFloatEqual(const float &num1, const float &num2);

}; // namespace winHelper
