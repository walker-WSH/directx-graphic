#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include <vector>
#include <d3d11.h>
#include <dxgi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <source_location>
#include <functional>

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

#define COMBINE2(a, b) a##b
#define COMBINE1(a, b) COMBINE2(a, b)
#define AUTO_GRAPHIC_CONTEXT(graphic) \
	IAutoGraphicContext COMBINE1(autoContext, __LINE__)(graphic, std::source_location::current())

// Note: If you want to create D2D target based on DX11, the texutre of DX11 must be B8G8R8A8_UNORM
static const auto D2D_COMPATIBLE_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

namespace graphic {

class IGraphicObject;
class IGraphicSession;
class IAutoGraphicContext;

using texture_handle = IGraphicObject *;
using display_handle = IGraphicObject *;
using shader_handle = IGraphicObject *;
using font_handle = IGraphicObject *;
using geometry_handle = IGraphicObject *;
using graphic_cb = long;

enum class TEXTURE_USAGE {
	UNKNOWN = 0,

	// it includes a shared handle. besides, it can also be the canvas for copying another texture.
	// for copying, you must note MipLevels of another texture should be same with canvs.
	CANVAS_TARGET,

	// for reading GPU
	READ_TEXTURE,

	// for writing data to GPU
	WRITE_TEXTURE,

	// it is for openning other shared texture
	SHARED_TEXTURE,

	// WARNING:
	// "MipLevels" of static image texture may be more than 1 (it is not a fixed value).
	// However, canvas's level is fixed with 1. So it may be unable to copy static texture to canvas texture.
	STATIC_IMAGE_FILE,
};

enum class VIDEO_FILTER_TYPE {
	VIDEO_FILTER_POINT = 0,
	VIDEO_FILTER_LINEAR,
	VIDEO_FILTER_ANISOTROPIC,
};

enum class MAP_TEXTURE_FEATURE {
	FOR_READ_TEXTURE = D3D11_MAP_READ,
	FOR_WRITE_TEXTURE = D3D11_MAP_WRITE_DISCARD,
};

enum class VERTEX_INPUT_TYPE {
	NORMAL = 0,
	COLOR,
	POSITION,
	SV_POSITION,
	TEXTURE_COORD,
};

enum class VIDEO_BLEND_TYPE {
	DISABLE = 0,
	NORMAL,
};

enum class TEXT_ALIGNMENT_TYPE {
	ALIGNMENT_NEAR = 0,
	ALIGNMENT_CENTER,
	ALIGNMENT_FAR,
};

enum class TEXT_WORD_WRAP {
	// Words are broken across lines to avoid text overflowing the layout box.
	WORD_WRAPPING_WRAP = DWRITE_WORD_WRAPPING_WRAP,

	// Only wrap whole words, never breaking words (emergency wrapping) when the
	// layout width is too small for even a single word.
	WORD_WRAPPING_WHOLE_WORD = DWRITE_WORD_WRAPPING_WHOLE_WORD,

	// Words are kept within the same line even when it overflows the layout box.
	// This option is often used with scrolling to reveal overflow text.
	WORD_WRAPPING_NO_WRAP_WITHOUT_ELLIPSIS = DWRITE_WORD_WRAPPING_NO_WRAP,

	// display "..." if word is cut.
	WORD_WRAPPING_NO_WRAP_WITH_ELLIPSIS = 20000,
};

enum class GEOMETRY_TYPE {
	BEZIER_CURVE = 0,
	MULP_POINTS,
};

enum class GEOMETRY_FRONT_END_STYLE {
	FRONT_END_OPEN = D2D1_FIGURE_END::D2D1_FIGURE_END_OPEN,
	FRONT_END_CLOSE = D2D1_FIGURE_END::D2D1_FIGURE_END_CLOSED,
};

enum class LINE_DASH_STYLE {
	LINE_NO_STYLE = D2D1_DASH_STYLE_FORCE_DWORD,

	// "------------------"
	LINE_SOLID = D2D1_DASH_STYLE_SOLID,

	// "- - - - - - - - - "
	LINE_DASH = D2D1_DASH_STYLE_DASH,

	// ".................."
	LINE_DOT = D2D1_DASH_STYLE_DOT,

	// "-.-.-.-.-.-.-.-.-."
	LINE_DASH_DOT = D2D1_DASH_STYLE_DASH_DOT,

	// "-..-..-..-..-..-.."
	LINE_DASH_DOT_DOT = D2D1_DASH_STYLE_DASH_DOT_DOT,
};

struct TextureInformation {
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	enum TEXTURE_USAGE usage = TEXTURE_USAGE::UNKNOWN;
};

struct DisplayInformation {
	HWND hWnd = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
};

struct ColorRGB {
	float red = 0.0; // [0.0, 1.0]
	float green = 0.0;
	float blue = 0.0;
};

struct ColorRGBA {
	float red = 0.0; // [0.0, 1.0]
	float green = 0.0;
	float blue = 0.0;
	float alpha = 1.0; // [0.0, 1.0]
};

struct VertexInputDesc {
	VERTEX_INPUT_TYPE type;
	uint32_t size;
};

struct TextureCopyRegion {
	uint32_t destLeft = 0;
	uint32_t destTop = 0;

	uint32_t srcLeft = 0;
	uint32_t srcRight = 0;
	uint32_t srcTop = 0;
	uint32_t srcBottom = 0;
};

struct ShaderInformation {
	std::wstring vsFile;
	std::wstring psFile;

	std::vector<VertexInputDesc> vertexDesc;
	uint32_t perVertexSize = 0;
	uint32_t vertexCount = 0;

	uint32_t vsBufferSize = 0;
	uint32_t psBufferSize = 0;
};

struct GraphicCardDesc {
	std::wstring graphicName = L"";
	std::string driverVersion = "";

	LUID adapterLuid = {0, 0};
	uint32_t vendorId = 0;
	uint32_t deviceId = 0;

	uint64_t dedicatedVideoMemory = 0;
	uint64_t dedicatedSystemMemory = 0;
	uint64_t sharedSystemMemory = 0;
};

struct TextFormatDesc {
	std::wstring fontName = L""; // default font  will be used if it is empty
	float fontSize = 15.f;
	bool bold = false;
	bool italic = false;
	float lineSpacing = 0.f;
	DWRITE_FONT_STRETCH stretchH = DWRITE_FONT_STRETCH_NORMAL;
	TEXT_ALIGNMENT_TYPE alignH = TEXT_ALIGNMENT_TYPE::ALIGNMENT_NEAR;
	TEXT_ALIGNMENT_TYPE alignV = TEXT_ALIGNMENT_TYPE::ALIGNMENT_NEAR;
	TEXT_WORD_WRAP wordWrap = TEXT_WORD_WRAP::WORD_WRAPPING_WHOLE_WORD;
};

class IGraphicObject {
public:
	virtual ~IGraphicObject() = default;

	virtual bool IsBuilt() = 0;

	virtual graphic_cb RegisterCallback(std::function<void(IGraphicObject *obj)> cbRebuilt,
					    std::function<void(IGraphicObject *obj)> cbReleased) = 0;
	virtual void UnregisterCallback(graphic_cb hdl) = 0;
	virtual void ClearCallback() = 0;

	virtual void SetUserData(void *data) = 0;
	virtual void *GetUserData() = 0;
};

class IGraphicCallback {
public:
	virtual ~IGraphicCallback() = default;

	virtual void OnGraphicError(HRESULT hr) = 0;
	virtual void OnRequestRebuild() = 0;
	virtual void OnBuildSuccessed(const DXGI_ADAPTER_DESC &desc) = 0;
};

class GRAPHIC_API IAutoGraphicContext {
public:
	IAutoGraphicContext(IGraphicSession *graphic, const std::source_location &location);
	virtual ~IAutoGraphicContext();

private:
	class impl;
	impl *self;
};

} // namespace graphic
