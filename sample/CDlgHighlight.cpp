// CDlgHighlight.cpp: 实现文件
//

#include "pch.h"
#include "CDlgHighlight.h"
#include "afxdialogex.h"
#include "resource.h"

extern IGraphicSession *pGraphic;
extern texture_handle texHightlight;

// CDlgHighlight 对话框

IMPLEMENT_DYNAMIC(CDlgHighlight, CDialogEx)

CDlgHighlight::CDlgHighlight(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_HIGHLIGHT, pParent) {}

CDlgHighlight::~CDlgHighlight() {}

void CDlgHighlight::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER5, m_sliderCLARITY);
	DDX_Control(pDX, IDC_SLIDER3, m_sliderSHADOWS);
	DDX_Control(pDX, IDC_SLIDER2, m_sliderHIGHLIGHTS);
	DDX_Control(pDX, IDC_SLIDER6, m_sliderRADIUS);
}

BEGIN_MESSAGE_MAP(CDlgHighlight, CDialogEx)
ON_WM_TIMER()
ON_WM_PAINT()
END_MESSAGE_MAP()

// CDlgHighlight 消息处理程序

BOOL CDlgHighlight::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	MoveWindow(0, 0, 800, 600);
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	m_sliderHIGHLIGHTS.SetRange(-100, 100);
	m_sliderHIGHLIGHTS.SetPos(1);

	m_sliderSHADOWS.SetRange(-100, 100);
	m_sliderSHADOWS.SetPos(1);

	m_sliderCLARITY.SetRange(-100, 100);
	m_sliderCLARITY.SetPos(1);

	m_sliderRADIUS.SetRange(0, 1000);
	m_sliderRADIUS.SetPos(125);

	SetTimer(20000, 100, nullptr);
	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void CDlgHighlight::OnTimer(UINT_PTR nIDEvent)
{
	Invalidate(FALSE);

	highlight = (float)m_sliderHIGHLIGHTS.GetPos() / 100.f;
	shadow = (float)m_sliderSHADOWS.GetPos() / 100.f;
	clarity = (float)m_sliderCLARITY.GetPos() / 100.f;
	redius = (float)m_sliderRADIUS.GetPos() / 100.f;

	CDialogEx::OnTimer(nIDEvent);
}

texture_handle edit_canvasEffect = nullptr;
texture_handle test_chromakey = nullptr;

void CDlgHighlight::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	AUTO_GRAPHIC_CONTEXT(pGraphic);

	if (!texHightlight)
		return;

	if (!edit_canvasTex) {
		test_chromakey = pGraphic->OpenImageTexture(L"testChromekey.png");

		auto info = pGraphic->GetTextureInfo(test_chromakey);
		info.usage = TEXTURE_USAGE::CANVAS_TARGET;
		info.format = D2D_COMPATIBLE_FORMAT;
		edit_canvasTex = pGraphic->CreateTexture(info);
		edit_canvasEffect = pGraphic->CreateTexture(info);

		edit_display = pGraphic->CreateDisplay(m_hWnd);
		pGraphic->SetDisplaySize(edit_display, rc.right, rc.right);

		if (pGraphic->BeginRenderCanvas(edit_canvasTex)) {
			RenderTexture(std::vector<texture_handle>{test_chromakey}, SIZE(info.width, info.height),
				      RECT(0, 0, info.width, info.height));
			pGraphic->EndRender();
		}
	}

	IGeometryInterface *d2d = pGraphic->OpenGeometryInterface(edit_canvasEffect);
	if (d2d) {
		d2d->DrawChromakey(edit_canvasTex, ColorRGB(76 / 255.f, 121.f / 255.f, 3.f / 255.f), redius / 10.f);
		pGraphic->CloseGeometryInterface(edit_canvasEffect);
	}

	if (pGraphic->BeginRenderWindow(edit_display, &d2d)) {
		pGraphic->SetBlendState(VIDEO_BLEND_TYPE::NORMAL);

		ColorRGBA bk(1, 1, 1, 1);
		pGraphic->ClearBackground(&bk);
		RenderTexture(std::vector<texture_handle>{edit_canvasEffect}, SIZE(rc.right, rc.bottom), rc);

		//d2d->DrawHighlight(edit_canvasEffect, highlight, shadow, clarity, redius);
		pGraphic->EndRender(d2d);
	}
}
