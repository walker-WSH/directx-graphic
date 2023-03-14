#include "WindowsPlatformHelper.h"

#pragma comment(lib, "Shlwapi.lib")

namespace winHelper {

std::wstring GetExeFullPath()
{
	WCHAR szFilePath[MAX_PATH] = {};
	GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
	return szFilePath;
}

std::wstring GetFileDirectory(HINSTANCE hInstance)
{
	wchar_t dir[MAX_PATH] = {};
	GetModuleFileNameW(hInstance, dir, MAX_PATH);
	PathRemoveFileSpecW(dir);
	return std::wstring(dir) + std::wstring(L"\\");
}

std::string ExtractFileName(const char *full_path)
{
	if (!full_path)
		return "";

	LPCSTR name = PathFindFileNameA(full_path);
	if (!name)
		return "";

	return name;
}

std::string ExtractExtension(const char *full_path)
{
	if (!full_path)
		return "";

	auto ret = PathFindExtensionA(full_path);
	if (!ret)
		return "";

	return ret;
}

bool IsFloatEqual(const float &num1, const float &num2)
{
	static const float equal = 0.000001f;
	return (fabs(num1 - num2) < equal);
}

}; // namespace winHelper
