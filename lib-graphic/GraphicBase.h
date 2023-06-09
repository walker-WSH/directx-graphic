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
#include <d2d1effects_2.h>
#include <d2d1helper.h>
#include <initguid.h>
#include <source_location>
#include <optional>

#include "ComPtr.hpp"
#include "StringConvert.h"
#include "WindowsPlatformHelper.h"
#include "IGraphicDefine.h"
#include "GraphicFormat.hpp"
#include "GraphicLog.h"

namespace graphic {

//----------------------------------------------------------------------------------
#define CHECK_DX_ERROR(hr, format, ...) DO_DX_LOG(hr, format, ##__VA_ARGS__)
#define DO_DX_LOG(hr, format, ...)                                                        \
	LOG_WARN("DX ERROR : 0X%X, called in %s:%u. " format, hr, __FUNCTION__, __LINE__, \
		 ##__VA_ARGS__)

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

DEFINE_GUID(D2D_CLSID_D2D1GaussianBlur, 0x1feb6d69, 0x2fe6, 0x4ac9, 0x8c, 0x58, 0x1d, 0x7f, 0x93,
	    0xe7, 0xa6, 0xa5);
DEFINE_GUID(D2D_CLSID_D2D1DirectionalBlur, 0x174319a6, 0x58e9, 0x49b2, 0xbb, 0x63, 0xca, 0xf2, 0xc8,
	    0x11, 0xa3, 0xdb);
DEFINE_GUID(D2D_CLSID_D2D1ChromaKey, 0x74C01F5B, 0x2A0D, 0x408C, 0x88, 0xE2, 0xC7, 0xA3, 0xC7, 0x19,
	    0x77, 0x42);
DEFINE_GUID(D2D_CLSID_D2D1HighlightsShadows, 0xCADC8384, 0x323F, 0x4C7E, 0xA3, 0x61, 0x2E, 0x2B,
	    0x24, 0xDF, 0x6E, 0xE4);

const static std::vector<D3D_FEATURE_LEVEL> featureLevels = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

enum class D2D_EFFECT_TYPE {
	D2D_EFFECT_GAUSSIAN_BLUR = 0,
	D2D_EFFECT_DIRECT_BLUR,
	D2D_EFFFECT_HIGHLIGHT,
	D2D_EFFFECT_CHROMAKEY,
};

class ID3DRenderTarget {
public:
	virtual ~ID3DRenderTarget() = default;
	virtual ID3D11RenderTargetView *D3DTarget(bool srgb) = 0;
};

class DX11GraphicSession;
class DX11GraphicBase : public IGraphicObject {
public:
	DX11GraphicBase(DX11GraphicSession &graphic, const char *name);
	virtual ~DX11GraphicBase();

	const char *GetObjectName() { return m_strName.c_str(); }

	virtual graphic_cb RegisterCallback(std::function<void(IGraphicObject *obj)> cbRebuilt,
					    std::function<void(IGraphicObject *obj)> cbReleased);
	virtual void UnregisterCallback(graphic_cb hdl);
	virtual void ClearCallback();

	virtual void SetUserData(void *data);
	virtual void *GetUserData();

	virtual bool BuildGraphic() = 0;
	virtual void ReleaseGraphic(bool isForRebuild) = 0;

protected:
	void NotifyReleaseEvent(bool isForRebuild);
	void NotifyRebuildEvent();

	struct GraphicCallback {
		graphic_cb id = 0;
		std::function<void(IGraphicObject *obj)> cbRebuilt;
		std::function<void(IGraphicObject *obj)> cbReleased;
	};

	DX11GraphicSession &m_graphicSession;
	const std::string m_strName;
	void *m_userData = nullptr;
	std::vector<GraphicCallback> m_callbacks;
};

} // namespace graphic
