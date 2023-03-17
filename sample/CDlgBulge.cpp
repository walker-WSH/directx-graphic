// CDlgBulge.cpp: 实现文件
//

#include "pch.h"
#include "CDlgBulge.h"
#include "afxdialogex.h"
#include "resource.h"

// CDlgBulge 对话框

IMPLEMENT_DYNAMIC(CDlgBulge, CDialogEx)

CDlgBulge::CDlgBulge(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_BULGE, pParent) {}

CDlgBulge::~CDlgBulge() {}

void CDlgBulge::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editCenterX);
	DDX_Control(pDX, IDC_EDIT2, m_editCenterY);
	DDX_Control(pDX, IDC_EDIT3, m_editRadius);
	DDX_Control(pDX, IDC_SLIDER1, m_sliderRadius);
	DDX_Control(pDX, IDC_CHECK3, m_checkReduce);
	DDX_Control(pDX, IDC_SLIDER2, m_sliderWeight);
}

BEGIN_MESSAGE_MAP(CDlgBulge, CDialogEx)
ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDlgBulge::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	MoveWindow(0, 0, 300, 400);

	m_editCenterX.SetWindowText(L"160");
	m_editCenterY.SetWindowText(L"240");

	m_sliderRadius.SetRange(0, 300);
	m_sliderRadius.SetPos(30);

	m_sliderWeight.SetRange(0, 100);
	m_sliderWeight.SetPos(20);

	SetTimer(20000, 100, nullptr);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

bool g_bReduce = false;
int g_nCenterX = 100;
int g_nCenterY = 100;
int g_nRadius = 40;
float g_fWeight = 30.f;
void CDlgBulge::OnTimer(UINT_PTR nIDEvent)
{
	CString text;

	g_bReduce = !!m_checkReduce.GetCheck();

	m_editCenterX.GetWindowText(text);
	g_nCenterX = _wtoi(text.GetBuffer());

	m_editCenterY.GetWindowText(text);
	g_nCenterY = _wtoi(text.GetBuffer());

	g_nRadius = m_sliderRadius.GetPos();
	g_fWeight = (float)m_sliderWeight.GetPos();

	CDialogEx::OnTimer(nIDEvent);
}
