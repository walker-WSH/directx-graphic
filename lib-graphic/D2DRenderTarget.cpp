#include "D2DRenderTarget.h"
#include "GraphicSessionImp.h"
#include "D2DTextFormat.h"
#include "D2DGeometry.h"
#include "D2DLineStyle.h"
#include "DX11Texture2D.h"

#define CHECK_D2D_RENDER_STATE(action)                                 \
	if (!m_bD2DRendering) {                                        \
		LOG_WARN("D2D is not during render %X", (void *)this); \
		assert(false && "not in render");                      \
		action;                                                \
	}                                                              \
	if (!IsInterfaceValid()) {                                     \
		assert(false && "D2D not built");                      \
		action;                                                \
	}

namespace graphic {

D2DRenderTarget::D2DRenderTarget(DX11GraphicSession &graphic, bool forSwapchain)
	: m_graphicSession(graphic), m_bIsForSwapchain(forSwapchain)
{
}

bool D2DRenderTarget::BeginDrawD2D(std::source_location location)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	if (m_bIsForSwapchain && !m_graphicSession.IsDuringRender()) {
		LOG_WARN("D2D is not used during render HWND %X from %s", this, location.function_name());
		assert(false);
	}

	if (m_bD2DRendering) {
		LOG_WARN("D2D is during render %X from %s", this, location.function_name());
		assert(false);
		return true;
	}

	if (!IsInterfaceValid())
		return false;

	m_bD2DRendering = true;
	m_pRenderTarget->BeginDraw();

	return true;
}

HRESULT D2DRenderTarget::EndDrawD2D(std::source_location location)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	if (m_bIsForSwapchain && !m_graphicSession.IsDuringRender()) {
		LOG_WARN("D2D is not used during render HWND %X from %s", this, location.function_name());
		assert(false);
	}

	if (!m_bD2DRendering) {
		LOG_WARN("D2D is not during render %X from %s", this, location.function_name());
		assert(false);
		return S_OK;
	}

	m_bD2DRendering = false;
	assert(m_pRenderTarget);

	if (!IsInterfaceValid()) {
		LOG_WARN("D2D is during render, but now target is released! %X from %s", this,
			 location.function_name());
		return S_FALSE;
	}

	auto ret = m_pRenderTarget->EndDraw();
	if (FAILED(ret)) {
		LOG_WARN("D2D EndDraw failed with %X %s ,from %s", ret,
			 (D2DERR_RECREATE_TARGET == ret) ? ", need to recreate" : "", location.function_name());
	}

	return ret;
}

bool D2DRenderTarget::IsInterfaceValid()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	return m_pRenderTarget && m_pSolidBrush && m_pD2DBitmapOnDXGI;
}

void D2DRenderTarget::ClearBackground(const ColorRGBA *bkClr)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	m_pRenderTarget->Clear(D2D1::ColorF(bkClr->red, bkClr->green, bkClr->blue, bkClr->alpha));
}

void D2DRenderTarget::DrawString(const wchar_t *text, size_t len, font_handle font, const ColorRGBA *color,
				 D2D1_RECT_F *region)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );
	assert(text);

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	if (!len)
		len = wcslen(text);

	D2D1_RECT_F temp = {0.f, 0.f, 0.f, 0.f};
	if (region) {
		temp = *region;
	} else {
		D2D1_SIZE_F size = m_pRenderTarget->GetSize();
		temp.right = size.width;
		temp.bottom = size.height;
	}

	CHECK_GRAPHIC_OBJECT_VALID(m_graphicSession, font, D2DTextFormat, realObj, return );
	m_pRenderTarget->DrawText(text, (uint32_t)len, realObj->m_pTextFormat, temp, brush);
}

void D2DRenderTarget::DrawLine(D2D1_POINT_2F start, D2D1_POINT_2F end, const ColorRGBA *color, FLOAT strokeWidth,
			       LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->DrawLine(start, end, brush, strokeWidth, m_graphicSession.GetLineStyle(style));
}

void D2DRenderTarget::FillEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *color)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->FillEllipse(ellipse, brush);
}

void D2DRenderTarget::DrawEllipse(const D2D1_ELLIPSE &ellipse, const ColorRGBA *color, FLOAT strokeWidth,
				  LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->DrawEllipse(ellipse, brush, strokeWidth, m_graphicSession.GetLineStyle(style));
}

void D2DRenderTarget::DrawRectangle(const D2D1_RECT_F &rect, const ColorRGBA *color, FLOAT strokeWidth,
				    LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->DrawRectangle(rect, brush, strokeWidth, m_graphicSession.GetLineStyle(style));
}

void D2DRenderTarget::FillRectangle(const D2D1_RECT_F &rect, const ColorRGBA *color)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->FillRectangle(rect, brush);
}

void D2DRenderTarget::FillRoundedRectangle(const D2D1_ROUNDED_RECT &roundedRect, const ColorRGBA *color)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->FillRoundedRectangle(roundedRect, brush);
}

void D2DRenderTarget::DrawRoundedRectangle(const D2D1_ROUNDED_RECT &roundedRect, const ColorRGBA *color,
					   FLOAT strokeWidth, LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	m_pRenderTarget->DrawRoundedRectangle(roundedRect, brush, strokeWidth, m_graphicSession.GetLineStyle(style));
}

void D2DRenderTarget::FillGeometry(geometry_handle path, const ColorRGBA *color)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	CHECK_GRAPHIC_OBJECT_VALID(m_graphicSession, path, D2DGeometry, realObj, return );
	assert(realObj->m_listPoints.size() >= 3);
	m_pRenderTarget->FillGeometry(realObj->m_pD2DGeometry, brush);
}

void D2DRenderTarget::DrawGeometry(geometry_handle path, const ColorRGBA *color, FLOAT strokeWidth,
				   LINE_DASH_STYLE style)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return );

	auto brush = GetSolidBrush(color);
	if (!brush)
		return;

	CHECK_GRAPHIC_OBJECT_VALID(m_graphicSession, path, D2DGeometry, realObj, return );
	assert(realObj->m_listPoints.size() >= 2);
	m_pRenderTarget->DrawGeometry(realObj->m_pD2DGeometry, brush, strokeWidth,
				      m_graphicSession.GetLineStyle(style));
}

void D2DRenderTarget::DrawImageWithGaussianBlur(texture_handle srcCanvas, float value,
						const D2D1_POINT_2F *targetOffset, const D2D1_RECT_F *imageRectangle,
						D2D1_INTERPOLATION_MODE interpolationMode,
						D2D1_COMPOSITE_MODE compositeMode)
{
	CHECK_GRAPHIC_OBJECT_VALID(m_graphicSession, srcCanvas, DX11Texture2D, srcTex, return );

	if (!IsInterfaceValid() || !srcTex->IsInterfaceValid()) {
		assert(false);
		return;
	}

	ID2D1DeviceContext *pD2DContext = m_graphicSession.D2DDeviceContext();
	auto pEffect = m_graphicSession.GetGaussianBlurEffect();

	assert(pD2DContext && pEffect);
	if (pD2DContext && pEffect) {
		pEffect->SetInput(0, srcTex->m_pD2DBitmapOnDXGI);
		pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, value);

		pD2DContext->SetTarget(m_pD2DBitmapOnDXGI);

		pD2DContext->BeginDraw();
		pD2DContext->SetTransform(D2D1::Matrix3x2F::Identity());
		pD2DContext->Clear(D2D1::ColorF::ColorF(0x00000000));
		pD2DContext->DrawImage(pEffect, targetOffset, imageRectangle, interpolationMode, compositeMode);
		auto hr = pD2DContext->EndDraw();
		pD2DContext->SetTarget(nullptr);

		if (hr == D2DERR_RECREATE_TARGET) {
			LOG_WARN("D2D request rebuild %X, D2DRenderTarget: %X", hr, this);
		}
	}
}

HRESULT D2DRenderTarget::FlushGeometry(std::source_location location)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	CHECK_D2D_RENDER_STATE(return S_FALSE);

	auto hr = EndDrawD2D(location);
	BeginDrawD2D();

	return hr;
}

bool D2DRenderTarget::BuildD2D(ComPtr<IDXGISurface1> sfc)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto dsProps = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	auto hr = m_graphicSession.D2DFactory()->CreateDxgiSurfaceRenderTarget(sfc.Get(), &dsProps,
									       m_pRenderTarget.Assign());
	if (FAILED(hr)) {
		LOG_WARN("CreateDxgiSurfaceRenderTarget failed with 0x%x, D2DRenderTarget: %X", hr, this);
		assert(false);
		return false;
	}

	m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	D2D1::ColorF d2dColor(0.f, 0.f, 0.f, 1.f);
	hr = m_pRenderTarget->CreateSolidColorBrush(d2dColor, m_pSolidBrush.Assign());
	if (FAILED(hr)) {
		LOG_WARN("CreateSolidColorBrush failed with 0x%x, D2DRenderTarget: %X", hr, this);
		assert(false);
		return false;
	}

	D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(D2D_COMPATIBLE_FORMAT, D2D1_ALPHA_MODE_PREMULTIPLIED);
	D2D1_BITMAP_OPTIONS options = D2D1_BITMAP_OPTIONS_TARGET;
	if (m_bIsForSwapchain) {
		// D2D::bitmap based on D3D::swaipchain can not be d2d texture for d2d::DrawImage
		options |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	}

	D2D1_BITMAP_PROPERTIES1 prt = D2D1::BitmapProperties1(options, pixelFormat);
	hr = m_graphicSession.D2DDeviceContext()->CreateBitmapFromDxgiSurface(sfc, &prt, &m_pD2DBitmapOnDXGI);
	if (FAILED(hr)) {
		LOG_WARN("CreateBitmapFromDxgiSurface failed with 0x%x, D2DRenderTarget: %X", hr, this);
		assert(false);
		return false;
	}

	return true;
}

void D2DRenderTarget::ReleaseD2D()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_pSolidBrush = nullptr;
	m_pRenderTarget = nullptr;
	m_pD2DBitmapOnDXGI = nullptr;
}

ComPtr<ID2D1SolidColorBrush> D2DRenderTarget::GetSolidBrush(const ColorRGBA *color)
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	assert(m_pSolidBrush);
	if (!m_pSolidBrush)
		return nullptr;

	D2D1_COLOR_F clr;
	clr.r = color->red;
	clr.g = color->green;
	clr.b = color->blue;
	clr.a = color->alpha;

	m_pSolidBrush->SetColor(clr);
	return m_pSolidBrush;
}

} // namespace graphic
