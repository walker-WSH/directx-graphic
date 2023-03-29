#include "pch.h"
#include "CGenText.h"
#include <memory>
#include <string>
#include "RenderHelper.h"

#pragma comment(lib, "gdiplus.lib")

extern texture_handle texForWrite;

Gdiplus::REAL fontsize = 140.f;
auto width = 1280;
auto height = 720;

std::wstring m_strText = L"123WSH \n?他们接受";

typedef std::shared_ptr<Gdiplus::Font> GDIP_FONT_PTR;
typedef std::shared_ptr<Gdiplus::FontFamily> FontFamily_PTR;
typedef std::shared_ptr<Gdiplus::StringFormat> STRING_FORMAT_PTR;
typedef std::shared_ptr<Gdiplus::Bitmap> GDIP_BMP_PTR;

class CAutoLoadGDIPlus {
	ULONG_PTR m_GdiplusToken;

public:
	CAutoLoadGDIPlus()
	{
		Gdiplus::GdiplusStartupInput StartupInput;
		GdiplusStartup(&m_GdiplusToken, &StartupInput, NULL);
	}

	virtual ~CAutoLoadGDIPlus() { Gdiplus::GdiplusShutdown(m_GdiplusToken); }
} g_gdip;

extern bool g_checkboxPreMultAlpha;
Gdiplus::PixelFormat getTextFormat()
{
	return PixelFormat32bppARGB;

	/*
	if (g_checkboxPreMultAlpha)
		return PixelFormat32bppPARGB;
	else
		return PixelFormat32bppARGB;
	*/
}

int _GetFontStyle()
{
	bool m_bBold = true;
	bool m_bItalic = false;
	bool m_bUnderline = false;
	bool m_bStrikeout = false;

	int nFontStyle = Gdiplus::FontStyleRegular;

	if (m_bBold)
		nFontStyle |= Gdiplus::FontStyleBold;

	if (m_bItalic)
		nFontStyle |= Gdiplus::FontStyleItalic;

	if (m_bUnderline)
		nFontStyle |= Gdiplus::FontStyleUnderline;

	if (m_bStrikeout)
		nFontStyle |= Gdiplus::FontStyleStrikeout;

	return nFontStyle;
}

STRING_FORMAT_PTR _CreateStringFormat()
{
	int formatFlags = Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoFitBlackBox |
			  Gdiplus::StringFormatFlagsMeasureTrailingSpaces;
	Gdiplus::StringFormat *fmt = new Gdiplus::StringFormat(formatFlags);

	Gdiplus::REAL tabs[3];
	tabs[0] = tabs[1] = tabs[2] = fontsize * 7;

	fmt->SetHotkeyPrefix(Gdiplus::HotkeyPrefixNone);
	fmt->SetTabStops(0, 3, tabs);
	fmt->SetAlignment(Gdiplus::StringAlignmentCenter);
	fmt->SetLineAlignment(Gdiplus::StringAlignmentCenter);
	fmt->SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	fmt->SetTrimming(Gdiplus::StringTrimmingNone);

	return STRING_FORMAT_PTR(fmt);
}

FontFamily_PTR _CreateFontFamily()
{
	FontFamily_PTR pFontFamily = FontFamily_PTR(new Gdiplus::FontFamily(L"Arial"));
	if (pFontFamily->GetLastStatus() == Gdiplus::Ok)
		return pFontFamily;

	assert(false);
	return FontFamily_PTR();
}

void *CreateTextFrame()
{
	AUTO_GRAPHIC_CONTEXT(pGraphic);

	int fontStyle = _GetFontStyle();
	STRING_FORMAT_PTR stringformat = _CreateStringFormat();
	FontFamily_PTR family = _CreateFontFamily();
	if (!family.get())
		return NULL;

	Gdiplus::Font font(family.get(), fontsize, fontStyle);
	Gdiplus::Status status = font.GetLastStatus();
	if (status != Gdiplus::Ok) {
		return NULL;
	}

	Gdiplus::RectF outLayout;
	outLayout.X = outLayout.Y = 0;
	outLayout.Width = (float)width;
	outLayout.Height = (float)height;

	Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(width, height, getTextFormat());
	if (!bmp)
		return NULL;

	GDIP_BMP_PTR pRet(bmp);
	status = bmp->GetLastStatus();
	if (status != Gdiplus::Ok) {
		return NULL;
	}

	Gdiplus::Graphics graphics(bmp);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

	float emSize = graphics.GetDpiX() * font.GetSize() / 72.f;
	Gdiplus::GraphicsPath path(Gdiplus::FillModeWinding);
	status = path.AddString(m_strText.c_str(), -1, family.get(), fontStyle, emSize, outLayout, stringformat.get());
	if (status != Gdiplus::Ok) {
		return NULL;
	}

	Gdiplus::Color clrTextGDIP(255, 0, 0);
	Gdiplus::SolidBrush brushText(clrTextGDIP);
	graphics.FillPath(&brushText, &path);

	Gdiplus::BitmapData BitmapData;
	Gdiplus::Rect rcImage = Gdiplus::Rect(0, 0, bmp->GetWidth(), bmp->GetHeight());
	status = bmp->LockBits(&rcImage, Gdiplus::ImageLockModeRead, bmp->GetPixelFormat(), &BitmapData);
	if (status != Gdiplus::Ok) {
		return NULL;
	}

	if (texForWrite) {
		pGraphic->DestroyGraphicObject(texForWrite);
		texForWrite = 0;
	}

	TextureInformation info;
	info.width = bmp->GetWidth();
	info.height = bmp->GetHeight();
	info.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	info.usage = TEXTURE_USAGE::WRITE_TEXTURE;

	texForWrite = pGraphic->CreateTexture(info);
	D3D11_MAPPED_SUBRESOURCE mapdata;
	if (pGraphic->MapTexture(texForWrite, MAP_TEXTURE_FEATURE::FOR_WRITE_TEXTURE, &mapdata)) {
		if (BitmapData.Stride == mapdata.RowPitch) {
			memmove(mapdata.pData, BitmapData.Scan0, (size_t)BitmapData.Stride * BitmapData.Height);
		} else {
			BYTE *src = (BYTE *)BitmapData.Scan0;
			BYTE *dest = (BYTE *)mapdata.pData;
			for (size_t i = 0; i < BitmapData.Height; i++) {
				auto len = min((UINT)BitmapData.Stride, mapdata.RowPitch);
				memmove(dest, src, (size_t)len);

				src += BitmapData.Stride;
				dest += mapdata.RowPitch;
			}
		}

		pGraphic->UnmapTexture(texForWrite);
	}

	bmp->UnlockBits(&BitmapData);
	return 0;
}
