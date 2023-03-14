#include "D2DTextFormat.h"
#include "GraphicSessionImp.h"

namespace graphic {

const std::wstring &GetDefaultFont()
{
	static std::wstring defaultFont = L"";
	if (defaultFont.empty()) {
		LOGFONT info;
		if (GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &info) != 0) {
			defaultFont = info.lfFaceName;
		} else {
			defaultFont = L"Arial";
		}
	}

	return defaultFont;
}

D2DTextFormat::D2DTextFormat(DX11GraphicSession &graphic, const TextFormatDesc &desc)
	: DX11GraphicBase(graphic, "D2D TextFormat"), m_textFormat(desc)
{
	if (m_textFormat.fontName.empty()) {
		CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
		m_textFormat.fontName = GetDefaultFont();
	}

	BuildGraphic();
}

bool D2DTextFormat::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto wf = m_graphicSession.D2DWriteFactory();

	DWRITE_FONT_WEIGHT bold = m_textFormat.bold ? DWRITE_FONT_WEIGHT_DEMI_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE style = m_textFormat.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
	auto hr = wf->CreateTextFormat(m_textFormat.fontName.c_str(), NULL, bold, style, m_textFormat.stretchH,
				       m_textFormat.fontSize, L"", &m_pTextFormat);
	if (FAILED(hr)) {
		LOG_WARN("CreateTextFormat failed with 0x%x, [%s] fontsize:%f D2DTextFormat:%X", hr,
			 str::w2u(m_textFormat.fontName.c_str()).c_str(), m_textFormat.fontSize, this);
		assert(false);
		return false;
	}

	m_pTextFormat->SetTextAlignment(SwitchAlignH(m_textFormat.alignH));
	m_pTextFormat->SetParagraphAlignment(SwitchAlignV(m_textFormat.alignV));

	if (m_textFormat.lineSpacing > 0.f) {
		// according to doc of MS, value of "baseline" should be 0.8f x "lineSpacing"
		m_pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, m_textFormat.lineSpacing,
					      m_textFormat.lineSpacing * 0.8f);
	} else {
		m_pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_DEFAULT, 0.f, 0.f);
	}

	switch (m_textFormat.wordWrap) {
	case TEXT_WORD_WRAP::WORD_WRAPPING_WRAP:
	case TEXT_WORD_WRAP::WORD_WRAPPING_WHOLE_WORD:
		m_pTextFormat->SetWordWrapping((DWRITE_WORD_WRAPPING)m_textFormat.wordWrap);
		break;

	case TEXT_WORD_WRAP::WORD_WRAPPING_NO_WRAP_WITHOUT_ELLIPSIS:
		m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		break;

	case TEXT_WORD_WRAP::WORD_WRAPPING_NO_WRAP_WITH_ELLIPSIS:
		m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		hr = wf->CreateEllipsisTrimmingSign(m_pTextFormat, m_pTextCutShow.Assign());
		if (SUCCEEDED(hr)) {
			DWRITE_TRIMMING sc_trimming = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
			m_pTextFormat->SetTrimming(&sc_trimming, m_pTextCutShow);
		}
		break;

	default:
		LOG_WARN("unknown word wrap : %d, D2DTextFormat: %X", (int)m_textFormat.wordWrap, this);
		assert(false);
		break;
	}

	return true;
}

void D2DTextFormat::ReleaseGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_pTextCutShow = nullptr;
	m_pTextFormat = nullptr;
}

bool D2DTextFormat::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	return m_pTextFormat;
}

DWRITE_TEXT_ALIGNMENT D2DTextFormat::SwitchAlignH(TEXT_ALIGNMENT_TYPE alg)
{
	switch (alg) {
	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_CENTER:
		return DWRITE_TEXT_ALIGNMENT_CENTER;

	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_FAR:
		return DWRITE_TEXT_ALIGNMENT_TRAILING;

	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_NEAR:
	default:
		return DWRITE_TEXT_ALIGNMENT_LEADING;
	}
}

DWRITE_PARAGRAPH_ALIGNMENT D2DTextFormat::SwitchAlignV(TEXT_ALIGNMENT_TYPE alg)
{
	switch (alg) {
	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_CENTER:
		return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_FAR:
		return DWRITE_PARAGRAPH_ALIGNMENT_FAR;

	case graphic::TEXT_ALIGNMENT_TYPE::ALIGNMENT_NEAR:
	default:
		return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
	}
}

} // namespace graphic
