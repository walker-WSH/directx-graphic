#pragma once
#include <IGraphicDefine.h>

namespace graphic {

class IGeometryInterface {
public:
	virtual ~IGeometryInterface() = default;

	virtual bool IsInterfaceValid() = 0;

	virtual void ClearBackground(const ColorRGBA *bkClr) = 0;

	virtual void DrawString(const wchar_t *str, size_t len, font_handle font, const ColorRGBA *clr,
				D2D1_RECT_F *rg = nullptr) = 0;

	virtual void DrawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, const ColorRGBA *clr, FLOAT stroke = 1.0f,
			      LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				 LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				   LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawRoundedRectangle(const D2D1_ROUNDED_RECT &rc, const ColorRGBA *clr, FLOAT stroke = 1.0f,
					  LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;
	virtual void DrawGeometry(geometry_handle path, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				  LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE) = 0;

	virtual void FillEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *clr) = 0;
	virtual void FillRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr) = 0;
	virtual void FillRoundedRectangle(const D2D1_ROUNDED_RECT &roundedRect, const ColorRGBA *clr) = 0;
	virtual void FillGeometry(geometry_handle path, const ColorRGBA *clr) = 0;

	virtual HRESULT FlushGeometry(std::source_location caller = std::source_location::current()) = 0;

	virtual void
	DrawImageWithGaussianBlur(texture_handle srcCanvas, float value = 50.f,
				  const D2D1_POINT_2F *destOffset = nullptr, const D2D1_RECT_F *srcRect = nullptr,
				  D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
				  D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER) = 0;

	virtual void DrawImageWithDirectBlur(texture_handle srcCanvas, float value = 50.f, float angle = 135.f) = 0;
};

} // namespace graphic
