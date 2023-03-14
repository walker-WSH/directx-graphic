#pragma once
#include <assert.h>
#include <optional>
#include <map>
#include "IGraphicDefine.h"
#include "IGraphicSession.h"

using namespace graphic;

enum class VIDEO_SHADER_TYPE {
	SHADER_TEXTURE = 0,
	SHADER_FILL_RECT,

	// yuv to rgb
	SHADER_I420_TO_RGB,
	SHADER_NV12_TO_RGB,
	SHADER_YUYV_TO_RGB,
	SHADER_UYVY_TO_RGB,

	// rgb to yuv
	SHADER_TO_Y_PLANE,
	SHADER_TO_UV_PLANE,
};

struct WorldVector {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

struct WorldDesc {
	std::optional<WorldVector> rotate; // angle, [0, 360]
	std::optional<WorldVector> move;
	std::optional<WorldVector> scale;
};

static const auto TEXTURE_VERTEX_COUNT = 4;
struct TextureVertexDesc {
	float x, y, z, w;
	float u, v;
};
struct ColorVertexDesc {
	float x, y, z, w;
};

extern IGraphicSession *pGraphic;
extern std::map<VIDEO_SHADER_TYPE, shader_handle> shaders;
void InitShader();

void FillTextureVertex(float left, float top, float right, float bottom, bool flipH, bool flipV, float cropLeft,
		       float cropTop, float cropRight, float cropBtm,
		       TextureVertexDesc outputVertex[TEXTURE_VERTEX_COUNT]);
void FillColorVertex(float left, float top, float right, float bottom,
		     ColorVertexDesc outputVertex[TEXTURE_VERTEX_COUNT]);

void TransposeMatrixWVP(const SIZE &canvas, bool convertCoord, WorldDesc wd, float outputMatrix[4][4]);

texture_handle getRotatedTexture(texture_handle tex, texture_handle &cv);

void RenderTexture(std::vector<texture_handle> texs, SIZE canvas, RECT drawDest);

// 这个渲染border的方法 性能太低 应该优化
// 方法1：连续性渲染若干个三角形 一次性渲染整个border
// 方法2：使用D2D（性能很好！）
void FillRectangleByDX11(SIZE canvas, RECT drawDest, ColorRGBA clr);
void RenderBorderByDX11(SIZE canvas, RECT drawDest, long borderSize, ColorRGBA clr);
