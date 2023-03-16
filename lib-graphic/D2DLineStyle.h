#pragma once
#include <GraphicBase.h>

namespace graphic {

class DX11GraphicSession;
class D2DRenderTarget;

struct D2DLineStyle : public DX11GraphicBase {
	friend class DX11GraphicSession;
	friend class D2DRenderTarget;

public:
	D2DLineStyle(DX11GraphicSession &graphic, LINE_DASH_STYLE style);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

private:
	LINE_DASH_STYLE m_style = LINE_DASH_STYLE::LINE_SOLID;
	ComPtr<ID2D1StrokeStyle> m_pStrokeStyle = nullptr;
};

} // namespace graphic
