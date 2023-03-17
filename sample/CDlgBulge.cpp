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
}

BEGIN_MESSAGE_MAP(CDlgBulge, CDialogEx)
ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDlgBulge::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_editCenterX.SetWindowText(L"500");
	m_editCenterY.SetWindowText(L"400");
	m_editRadius.SetWindowText(L"100");

	m_sliderRadius.SetRange(0, 300);
	m_sliderRadius.SetPos(10);

	SetTimer(20000, 100, nullptr);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

int g_nCenterX = 100;
int g_nCenterY = 100;
int g_nRadius = 40;
void CDlgBulge::OnTimer(UINT_PTR nIDEvent)
{
	CString text;

	m_editCenterX.GetWindowText(text);
	g_nCenterX = _wtoi(text.GetBuffer());

	m_editCenterY.GetWindowText(text);
	g_nCenterY = _wtoi(text.GetBuffer());

	g_nRadius = m_sliderRadius.GetPos();

	CDialogEx::OnTimer(nIDEvent);
}
