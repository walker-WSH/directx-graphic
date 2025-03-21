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
#include <optional>
#include <directxmath.h>

using namespace DirectX; // for directxmath.h

#ifdef GRAPHIC_API_EXPORTS
#define GRAPHIC_API __declspec(dllexport)
#else
#define GRAPHIC_API __declspec(dllimport)
#endif

#define COMBINE2(a, b) a##b
#define COMBINE1(a, b) COMBINE2(a, b)
#define AUTO_GRAPHIC_CONTEXT(graphic)                                \
	IAutoGraphicContext COMBINE1(autoContext, __LINE__)(graphic, \
							    std::source_location::current())

namespace graphic {

class IGraphicObject;
class IGraphicSession;
class IAutoGraphicContext;

using texture_handle = IGraphicObject *;
using display_handle = IGraphicObject *;
using shader_handle = IGraphicObject *;
using buffer_handle = IGraphicObject *;
using font_handle = IGraphicObject *;
using geometry_handle = IGraphicObject *;
using graphic_cb = long;

static auto CREATE_TEXTURE_FLAG_SHARED_MUTEX = 1 << 0;
static auto CREATE_TEXTURE_FLAG_GDI_SHARED = 1 << 1;

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

	STATIC_IMAGE_FILE,

	// it includes 6 subresource
	CUBE_TEXTURE,
};

enum class VIDEO_FILTER_TYPE {
	VIDEO_FILTER_POINT = 0,
	VIDEO_FILTER_LINEAR,
	VIDEO_FILTER_ANISOTROPIC,
	VIDEO_FILTER_LINEAR_REPEAT,
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
	BLEND_DISABLED = 0,
	BLEND_NORMAL,        // Src_rgb * Src_a + Canvas_rgb * (1 - Src_a)
	BLEND_PREMULTIPLIED, // Src_rgb + Canvas_rgb * (1 - Src_a)
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

enum class GRAPHIC_FORMAT {
	GF_UNSUPPORTED = 0,

	// for SRGB
	GF_RGBA = DXGI_FORMAT_R8G8B8A8_TYPELESS,
	GF_BGRX = DXGI_FORMAT_B8G8R8X8_TYPELESS,
	GF_BGRA = DXGI_FORMAT_B8G8R8A8_TYPELESS,

	GF_R8_UNORM = DXGI_FORMAT_R8_UNORM,
	GF_R8G8_UNORM = DXGI_FORMAT_R8G8_UNORM,
	GF_RGBA_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,
	GF_BGRX_UNORM = DXGI_FORMAT_B8G8R8X8_UNORM,
	GF_BGRA_UNORM = DXGI_FORMAT_B8G8R8A8_UNORM,
};

enum class WORLD_TYPE {
	VECTOR_UNKNOWN = 0,
	VECTOR_MOVE,
	VECTOR_SCALE,
	VECTOR_ROTATE,
};

struct WorldVector {
	WORLD_TYPE type = WORLD_TYPE::VECTOR_UNKNOWN;

	// x/y/z presents angle value when type is VECTOR_ROTATE
	std::optional<float> x;
	std::optional<float> y;
	std::optional<float> z;
};

struct CameraDesc {
	XMVECTOR eyePos = {0.0f, 0.0f, -1.0f};
	XMVECTOR eyeUpDir = {0.0f, 1.0f, 0.0f};
	XMVECTOR lookAt = {0.0f, 0.0f, 0.0f};
};

struct TextureInformation {
	uint32_t width = 0;
	uint32_t height = 0;
	enum DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	enum TEXTURE_USAGE usage = TEXTURE_USAGE::UNKNOWN;
	UINT sampleCount = 1;
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

struct BufferDesc {
	uint32_t itemCount = 0;
	uint32_t sizePerItem = 0;
	D3D11_BIND_FLAG bufferType;
};

struct VertexInputDesc {
	VERTEX_INPUT_TYPE type;
	uint32_t size;
};

struct TextureCopyRegion {
	uint32_t destLeft = 0;
	uint32_t destTop = 0;
	uint32_t destArraySliceIndex = 0; // [0, 5]

	uint32_t srcLeft = 0;
	uint32_t srcTop = 0;
	uint32_t srcRight = 0;
	uint32_t srcBottom = 0;
};

struct ShaderInformation {
	std::wstring vsFile;
	std::wstring psFile;

	std::vector<VertexInputDesc> vertexDesc;

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

	virtual graphic_cb
	RegisterCallback(std::function<void(IGraphicObject *obj)> cbRebuilt,
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
