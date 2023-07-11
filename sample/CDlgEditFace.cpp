// CDlgEditFace.cpp: 实现文件
//

#include "pch.h"
#include "CDlgEditFace.h"
#include "afxdialogex.h"
#include "resource.h"
#include "RenderHelper.h"
#include "MFCDemoDlg.h"

// CDlgEditFace 对话框

extern IGraphicSession *pGraphic;
extern texture_handle texGrid;

extern float g_nMoveRadius;
extern float g_nMoveCurve; // default 1

IMPLEMENT_DYNAMIC(CDlgEditFace, CDialogEx)

CDlgEditFace::CDlgEditFace(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG1, pParent) {}

CDlgEditFace::~CDlgEditFace() {}

BEGIN_MESSAGE_MAP(CDlgEditFace, CDialogEx)
ON_WM_NCHITTEST()
ON_WM_TIMER()
ON_WM_PAINT()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONUP()
ON_WM_LBUTTONDOWN()
ON_BN_CLICKED(IDC_BUTTON3, &CDlgEditFace::OnBnClickedButton3)
ON_BN_CLICKED(IDC_BUTTON2, &CDlgEditFace::OnBnClickedButton2)
END_MESSAGE_MAP()

void CDlgEditFace::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_btnPos);
}

BOOL CDlgEditFace::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	MoveWindow(0, 0, 300, 400);
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	SetTimer(20000, 100, nullptr);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

LRESULT CDlgEditFace::OnNcHitTest(CPoint pt)
{
	RECT rcWindow;
	::GetWindowRect(m_hWnd, &rcWindow);

	if (pt.y <= (rcWindow.top + DRAGE_REGION_SIZE))
		return HTCAPTION;

	return __super::OnNcHitTest(pt);
}

void CDlgEditFace::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_bResized) {
		if (texGrid) {
			AUTO_GRAPHIC_CONTEXT(pGraphic);

			auto info = pGraphic->GetTextureInfo(texGrid);
			info.usage = TEXTURE_USAGE::CANVAS_TARGET;

			MoveWindow(0, 0, info.width, info.height);

			edit_bulgeTex = pGraphic->CreateTexture(info);

			edit_display = pGraphic->CreateDisplay(m_hWnd);
			pGraphic->SetDisplaySize(edit_display, info.width, info.height);

			m_bResized = true;
		}

		return;
	}

	Invalidate(FALSE);

	CDialogEx::OnTimer(nIDEvent);
}

void CDlgEditFace::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_ptOrigin = m_ptTarget = point;
	m_bEditing = true;
	SetCapture();

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CDlgEditFace::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bEditing) {
		m_ptTarget = point;
		m_bEditing = false;

		FinishedEditor info;
		info.m_ptOrigin = m_ptOrigin;
		info.m_ptTarget = m_ptTarget;
		;
		info.g_nMoveRadius = g_nMoveRadius;
		info.g_nMoveCurve = g_nMoveCurve;
		m_finishedList.push_back(info);

		ReleaseCapture();
		Invalidate(FALSE);
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CDlgEditFace::OnMouseMove(UINT nFlags, CPoint point)
{
	CString text;
	text.Format(L"%d,%d", point.x, point.y);
	m_btnPos.SetWindowText(text);

	if (m_bEditing) {
		m_ptTarget = point;
		Invalidate(FALSE);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CDlgEditFace::OnBnClickedButton2()
{
	m_finishedList.clear();
}

void CDlgEditFace::OnBnClickedButton3()
{
	if (!m_finishedList.empty()) {
		auto itr = m_finishedList.end();
		--itr;
		m_finishedList.erase(itr);
	}
}

void CDlgEditFace::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (!edit_bulgeTex)
		return;

	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	auto editorList = m_finishedList;
	if (m_bEditing) {
		FinishedEditor info;
		info.m_ptOrigin = m_ptOrigin;
		info.m_ptTarget = m_ptTarget;
		info.g_nMoveRadius = g_nMoveRadius;
		info.g_nMoveCurve = g_nMoveCurve;

		editorList.push_back(info);
	}

	assert(editorList.size() <= MAX_SHIFT_ITEM);

	auto texInfo = pGraphic->GetTextureInfo(texGrid);

	ShiftParam moveParam;
	moveParam.texWidth = texInfo.width;
	moveParam.texHeight = texInfo.height;
	moveParam.count = (int)editorList.size();
	for (int i = 0; i < moveParam.count; i++) {
		const auto &item = editorList[i];
		auto &dest = moveParam.items[i];
		dest.originPositionX = item.m_ptOrigin.x;
		dest.originPositionY = item.m_ptOrigin.y;
		dest.targetPositionX = item.m_ptTarget.x;
		dest.targetPositionY = item.m_ptTarget.y;
		dest.radius = item.g_nMoveRadius;
		dest.curve = item.g_nMoveCurve;
	}

	if (pGraphic->BeginRenderCanvas(edit_bulgeTex)) {
		RenderShiftTexture(std::vector<texture_handle>{texGrid},
				   SIZE(texInfo.width, texInfo.height),
				   RECT(0, 0, texInfo.width, texInfo.height), &moveParam);
		pGraphic->EndRender();
	}

	IGeometryInterface *d2d = 0;
	if (pGraphic->BeginRenderWindow(edit_display, &d2d)) {
		auto tex = editorList.empty() ? texGrid : edit_bulgeTex;
		RenderTexture(std::vector<texture_handle>{tex}, SIZE(rc.right, rc.bottom), rc);

		if (d2d) {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			D2D1_ELLIPSE elp;
			elp.point = {(float)pt.x, (float)pt.y};
			elp.radiusX = elp.radiusY = g_nMoveRadius;

			ColorRGBA red(1.f, 0, 0, 1.f);

			d2d->DrawEllipse(elp, &red);
		}

		pGraphic->EndRender(d2d);
	}
}
