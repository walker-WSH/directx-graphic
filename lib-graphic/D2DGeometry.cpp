#include "D2DGeometry.h"
#include "GraphicSessionImp.h"
#include <math.h>

namespace graphic {

static const auto MIN_POINTS_NUM = 2;

D2DGeometry::D2DGeometry(DX11GraphicSession &graphic, const std::vector<D2D1_POINT_2F> &points, GEOMETRY_TYPE type,
			 GEOMETRY_FRONT_END_STYLE style)
	: DX11GraphicBase(graphic, "D2D Geometry"), m_listPoints(points), m_pathType(type), m_frontEndStyle(style)
{
	BuildGraphic();
}

bool D2DGeometry::BuildGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);

	auto pointCount = m_listPoints.size();
	if (pointCount < MIN_POINTS_NUM) {
		LOG_WARN("there is no point %X", this);
		return false;
	}

	auto d2d1Factory = m_graphicSession.D2DFactory();
	if (!d2d1Factory) {
		LOG_WARN("there is no D2DFactory %X", this);
		return false;
	}

	ComPtr<ID2D1PathGeometry> pathGeometry;
	auto hr = d2d1Factory->CreatePathGeometry(pathGeometry.Assign());
	if (FAILED(hr)) {
		LOG_WARN("CreatePathGeometry failed with %X, %X", hr, this);
		return false;
	}

	ComPtr<ID2D1GeometrySink> pSink;
	hr = pathGeometry->Open(pSink.Assign());
	if (FAILED(hr)) {
		LOG_WARN("ID2D1GeometrySink failed with %X, %X", hr, this);
		return false;
	}

	if (m_pathType == GEOMETRY_TYPE::BEZIER_CURVE) {
		pSink->BeginFigure(m_listPoints[0], D2D1_FIGURE_BEGIN_FILLED);

		D2D1_BEZIER_SEGMENT seg;
		for (size_t i = 1; (i + 2) < pointCount; i += 3) {
			seg.point1 = m_listPoints[i];
			seg.point2 = m_listPoints[i + 1];
			seg.point3 = m_listPoints[i + 2];
			pSink->AddBezier(seg);
		}

	} else {
		pSink->BeginFigure(m_listPoints[0], D2D1_FIGURE_BEGIN_FILLED);
		pSink->AddLines(&(m_listPoints[1]), (UINT32)pointCount - 1);
	}

	pSink->EndFigure((D2D1_FIGURE_END)m_frontEndStyle);
	pSink->Close();

	m_pD2DGeometry = pathGeometry;
	return true;
}

void D2DGeometry::ReleaseGraphic()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	m_pD2DGeometry = nullptr;
}

bool D2DGeometry::IsBuilt()
{
	CHECK_GRAPHIC_CONTEXT_EX(m_graphicSession);
	return m_pD2DGeometry;
}

} // namespace graphic
