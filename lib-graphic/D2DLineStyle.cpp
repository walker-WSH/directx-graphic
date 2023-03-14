#include "D2DLineStyle.h"
#include "GraphicSessionImp.h"

namespace graphic {

D2DLineStyle::D2DLineStyle(DX11GraphicSession &graphic, LINE_DASH_STYLE style)
	: DX11GraphicBase(graphic, "D2D LineStyle"), m_style(style)
{
	// Here we will not start to build, because engine may be not ready.
}

bool D2DLineStyle::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto hr = m_graphicSession.D2DFactory()->CreateStrokeStyle(
		D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
					    D2D1_LINE_JOIN_ROUND, 10.0f, (D2D1_DASH_STYLE)m_style, 0.0f),
		nullptr, 0, m_pStrokeStyle.Assign());
	if (FAILED(hr)) {
		LOG_WARN("CreateStrokeStyle failed with 0x%x, m_style:%d, D2DLineStyle:%X", hr, (int)m_style, this);
		ReleaseGraphic();
		assert(false);
		return false;
	}

	return true;
}

void D2DLineStyle::ReleaseGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_pStrokeStyle = nullptr;
}

bool D2DLineStyle::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	return m_pStrokeStyle;
}

} // namespace graphic
