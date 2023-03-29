#pragma once
#include "IGraphicDefine.h"
#include <assert.h>
#include <vector>

namespace graphic {

static inline DXGI_FORMAT GetResourceViewFormat(GRAPHIC_FORMAT format)
{
	switch (format) {
	case GRAPHIC_FORMAT::GF_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case GRAPHIC_FORMAT::GF_BGRX:
		return DXGI_FORMAT_B8G8R8X8_UNORM;
	case GRAPHIC_FORMAT::GF_BGRA:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	default:
		return (DXGI_FORMAT)format;
	}
}

static inline DXGI_FORMAT GetResourceViewLinearFormat(GRAPHIC_FORMAT format)
{
	switch (format) {
	case GRAPHIC_FORMAT::GF_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case GRAPHIC_FORMAT::GF_BGRX:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	case GRAPHIC_FORMAT::GF_BGRA:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	default:
		return (DXGI_FORMAT)format;
	}
}

static const std::vector<GRAPHIC_FORMAT> formats = {
	GRAPHIC_FORMAT::GF_RGBA,       GRAPHIC_FORMAT::GF_BGRX,       GRAPHIC_FORMAT::GF_BGRA,
	GRAPHIC_FORMAT::GF_R8_UNORM,   GRAPHIC_FORMAT::GF_R8G8_UNORM, GRAPHIC_FORMAT::GF_RGBA_UNORM,
	GRAPHIC_FORMAT::GF_BGRX_UNORM, GRAPHIC_FORMAT::GF_BGRA_UNORM,
};

static inline GRAPHIC_FORMAT GetGraphicFormat(DXGI_FORMAT format)
{
	for (const auto &item : formats) {
		if ((int)item == (int)format) {
			return item;
		}
	}

	assert(false);
	return GRAPHIC_FORMAT::GF_UNSUPPORTED;
}

} // namespace graphic
