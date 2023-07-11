#pragma once
#include <lib-graphic/IGraphicDefine.h>

/*
Note: If you want to use D2D based on DX11, the texutre of DX11 must be:
DXGI_FORMAT_A8_UNORM
DXGI_FORMAT_R8G8B8A8_UNORM
DXGI_FORMAT_B8G8R8A8_UNORM
DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
DXGI_FORMAT_R16G16B16A16_UNORM
DXGI_FORMAT_R32G32B32A32_FLOAT
DXGI_FORMAT_R16G16B16A16_FLOAT
*/

namespace graphic {

class IGeometryInterface {
public:
	virtual ~IGeometryInterface() = default;

	virtual bool IsInterfaceValid() = 0;

	virtual void ClearBackground(const ColorRGBA *bkClr) = 0;

	virtual void DrawString(const wchar_t *str, size_t len, font_handle font,
				const ColorRGBA *clr, D2D1_RECT_F *rg = nullptr) = 0;

	virtual void DrawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, const ColorRGBA *clr,
			      FLOAT stroke = 1.0f,
			      LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *clr,
				 FLOAT stroke = 1.0f,
				 LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr,
				   FLOAT stroke = 1.0f,
				   LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void
	DrawRoundedRectangle(const D2D1_ROUNDED_RECT &rc, const ColorRGBA *clr, FLOAT stroke = 1.0f,
			     LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawGeometry(geometry_handle path, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				  LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;

	virtual void FillEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *clr) = 0;
	virtual void FillRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr) = 0;
	virtual void FillRoundedRectangle(const D2D1_ROUNDED_RECT &roundedRect,
					  const ColorRGBA *clr) = 0;
	virtual void FillGeometry(geometry_handle path, const ColorRGBA *clr) = 0;

	virtual bool
	FlushGeometry(std::source_location caller = std::source_location::current()) = 0;

	virtual void
	DrawGaussianBlur(texture_handle srcCanvas, float value = 50.f,
			 const D2D1_POINT_2F *destOffset = nullptr,
			 const D2D1_RECT_F *srcRect = nullptr,
			 D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
			 D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER) = 0;

	virtual void
	DrawDirectBlur(texture_handle srcCanvas, float value = 50.f, float angle = 135.f,
		       const D2D1_POINT_2F *destOffset = nullptr,
		       const D2D1_RECT_F *srcRect = nullptr,
		       D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
		       D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER) = 0;

	// highlight/shadows/clarity [-1.f, 1.f]
	// radius [0.f, 10.f]
	virtual void
	DrawHighlight(texture_handle srcCanvas, float highlight = 0.f, float shadows = 0.f,
		      float clarity = 0.f, float radius = 1.25f,
		      const D2D1_POINT_2F *destOffset = nullptr,
		      const D2D1_RECT_F *srcRect = nullptr,
		      D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
		      D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER) = 0;

	// tolerance [0.f, 1.f]
	// its smooth effect is worse than OBS
	virtual void DrawChromakey(texture_handle srcCanvas, ColorRGB clrKey,
				   float tolerance = 0.1f, bool invertAlpha = false,
				   bool smooth = true) = 0;
};

} // namespace graphic
