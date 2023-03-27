#pragma once
#include <vector>
#include <memory>
#include <GraphicBase.h>
#include <IGeometryInterface.h>

namespace graphic {

class DX11GraphicSession;

class D2DRenderTarget : public IGeometryInterface {
	friend class DX11GraphicSession;

public:
	D2DRenderTarget(DX11GraphicSession &graphic, bool forSwapchain);

	virtual bool IsInterfaceValid();

	virtual void ClearBackground(const ColorRGBA *bkClr);

	virtual void DrawString(const wchar_t *str, size_t len, font_handle font, const ColorRGBA *clr,
				D2D1_RECT_F *rg = nullptr);

	virtual void DrawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, const ColorRGBA *clr, FLOAT stroke = 1.0f,
			      LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE);
	virtual void DrawEllipse(const D2D1_ELLIPSE &eps, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				 LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE);
	virtual void DrawRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				   LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE);
	virtual void DrawRoundedRectangle(const D2D1_ROUNDED_RECT &rc, const ColorRGBA *clr, FLOAT stroke = 1.0f,
					  LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE);
	virtual void DrawGeometry(geometry_handle path, const ColorRGBA *clr, FLOAT stroke = 1.0f,
				  LINE_DASH_STYLE style = LINE_DASH_STYLE::LINE_NO_STYLE);

	virtual void FillEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *clr);
	virtual void FillRectangle(const D2D1_RECT_F &rect, const ColorRGBA *clr);
	virtual void FillRoundedRectangle(const D2D1_ROUNDED_RECT &roundedRect, const ColorRGBA *clr);
	virtual void FillGeometry(geometry_handle path, const ColorRGBA *clr);

	virtual bool FlushGeometry(std::source_location location = std::source_location::current());

	virtual void DrawGaussianBlur(texture_handle srcCanvas, float value = 50.f,
				      const D2D1_POINT_2F *targetOffset = nullptr,
				      const D2D1_RECT_F *imageRectangle = nullptr,
				      D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
				      D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER);

	virtual void DrawDirectBlur(texture_handle srcCanvas, float value = 50.f, float angle = 135.f);

	virtual void DrawHighlight(texture_handle srcCanvas, float highlight = 0.f, float shadows = 0.f,
				   float clarity = 0.f, float radius = 1.25f, const D2D1_POINT_2F *destOffset = nullptr,
				   const D2D1_RECT_F *srcRect = nullptr,
				   D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
				   D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER);

protected:
	bool BeginDrawD2D(std::source_location location = std::source_location::current());
	HRESULT EndDrawD2D(std::source_location location = std::source_location::current());

	bool BuildD2D(ComPtr<IDXGISurface1> sfc);
	void ReleaseD2D();

	ComPtr<ID2D1SolidColorBrush> GetSolidBrush(const ColorRGBA *clr);

private:
	struct BrushInfo {
		ColorRGBA color = {0.f, 0.f, 0.f, 1.f};
		ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	};

	DX11GraphicSession &m_graphicSession;
	const bool m_bIsForSwapchain = false;

	std::atomic<bool> m_bD2DRendering = false;
	ComPtr<ID2D1Bitmap1> m_pD2DBitmapOnDXGI = nullptr;
	ComPtr<ID2D1RenderTarget> m_pRenderTarget = nullptr;
	ComPtr<ID2D1SolidColorBrush> m_pSolidBrush = nullptr;
};

} // namespace graphic
