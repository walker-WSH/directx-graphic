#pragma once
#include "IGraphicDefine.h"

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

} // namespace graphic
