#pragma once
#include <GraphicBase.h>

namespace graphic {

class DX11GraphicSession;
class D2DRenderTarget;

struct D2DGeometry : public DX11GraphicBase {
	friend class DX11GraphicSession;
	friend class D2DRenderTarget;

public:
	D2DGeometry(DX11GraphicSession &graphic, const std::vector<D2D1_POINT_2F> &points,
		    GEOMETRY_TYPE type, GEOMETRY_FRONT_END_STYLE style);

	virtual bool BuildGraphic();
	virtual void ReleaseGraphic(bool isForRebuild);
	virtual bool IsBuilt();

private:
	const std::vector<D2D1_POINT_2F> m_listPoints;
	GEOMETRY_TYPE m_pathType = GEOMETRY_TYPE::MULP_POINTS;
	GEOMETRY_FRONT_END_STYLE m_frontEndStyle = GEOMETRY_FRONT_END_STYLE::FRONT_END_CLOSE;
	ComPtr<ID2D1PathGeometry> m_pD2DGeometry = nullptr;
};

} // namespace graphic
