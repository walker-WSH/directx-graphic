#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <assert.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <d2d1effects.h>
#include <d2d1helper.h>
#include <initguid.h>
#include <source_location>
#include <optional>

#include "ComPtr.hpp"
#include "StringConvert.h"
#include "WindowsPlatformHelper.h"
#include "IGraphicDefine.h"

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

//----------------------------------------------------------------------------------
static const auto SWAPCHAIN_TEXTURE_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

#define CHECK_DX_ERROR(hr, format, ...) DO_DX_LOG(hr, format, ##__VA_ARGS__)
#define DO_DX_LOG(hr, format, ...) \
	LOG_WARN("DX ERROR : 0X%X, called in %s:%u. " format, hr, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define CHECK_GRAPHIC_OBJECT_VALID(ses, hdl, type, varial, action)                                 \
	if (!hdl) {                                                                                \
		LOG_WARN("empty graphic object for '%s'", #type);                                  \
		assert(false);                                                                     \
		action;                                                                            \
	}                                                                                          \
	if (!ses.IsGraphicObjectAlive(hdl)) {                                                      \
		LOG_WARN("you are using deleted graphic object 0x%X", (void *)hdl);                \
		assert(false);                                                                     \
		action;                                                                            \
	}                                                                                          \
	auto varial = dynamic_cast<type *>(hdl);                                                   \
	if (!varial) {                                                                             \
		LOG_WARN("wrong type for graphic object 0X%X, expected '%s'", (void *)hdl, #type); \
		assert(false);                                                                     \
		action;                                                                            \
	}                                                                                          \
	if (!varial->IsBuilt()) {                                                                  \
		LOG_WARN("graphic object is not built 0X%X", (void *)hdl);                         \
		assert(false);                                                                     \
		action;                                                                            \
	}

DEFINE_GUID(D2D_CLSID_D2D1GaussianBlur, 0x1feb6d69, 0x2fe6, 0x4ac9, 0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5);
DEFINE_GUID(D2D_CLSID_D2D1DirectionalBlur, 0x174319a6, 0x58e9, 0x49b2, 0xbb, 0x63, 0xca, 0xf2, 0xc8, 0x11, 0xa3, 0xdb);

const static std::vector<D3D_FEATURE_LEVEL> featureLevels = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

enum class D2D_EFFECT_TYPE {
	D2D_EFFECT_GAUSSIAN_BLUR = 0,
	D2D_EFFECT_DIRECT_BLUR,
};

class DX11GraphicSession;
class DX11GraphicBase : public IGraphicObject {
public:
	DX11GraphicBase(DX11GraphicSession &graphic, const char *name);
	virtual ~DX11GraphicBase();

	const char *GetObjectName() { return m_strName.c_str(); }

	virtual void SetUserData(void *data);
	virtual void *GetUserData();
	virtual bool BuildGraphic() = 0;
	virtual void ReleaseGraphic() = 0;

protected:
	DX11GraphicSession &m_graphicSession;
	const std::string m_strName;
	void *m_userData = nullptr;
};

} // namespace graphic
