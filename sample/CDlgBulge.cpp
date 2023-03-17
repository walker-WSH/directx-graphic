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
	DDX_Control(pDX, IDC_EDIT4, m_editOrgX);
	DDX_Control(pDX, IDC_EDIT5, m_editOrgY);
	DDX_Control(pDX, IDC_EDIT6, m_editTgtX);
	DDX_Control(pDX, IDC_EDIT7, m_editTgtY);
	DDX_Control(pDX, IDC_SLIDER3, m_sliderMoveRadius);
	DDX_Control(pDX, IDC_SLIDER4, m_sliderMoveWeight);
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

	m_editOrgX.SetWindowText(L"65");
	m_editOrgY.SetWindowText(L"410");

	m_editTgtX.SetWindowText(L"170");
	m_editTgtY.SetWindowText(L"350");

	m_sliderMoveRadius.SetRange(0, 300);
	m_sliderMoveRadius.SetPos(80);

	m_sliderMoveWeight.SetRange(1, 100);
	m_sliderMoveWeight.SetPos(1);

	SetTimer(20000, 100, nullptr);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

bool g_bReduce = false;
int g_nCenterX = 100;
int g_nCenterY = 100;
int g_nRadius = 40;
float g_fWeight = 30.f;

int g_nOrigX = 0;
int g_nOrigY = 0;
int g_nTgtX = 0;
int g_nTgtY = 0;
float g_nMoveRadius = 0.f;
float g_nMoveCurve = 1.f; // default 1

int getEditNum(CEdit &editCenterX)
{
	CString text;
	editCenterX.GetWindowText(text);
	return _wtoi(text.GetBuffer());
}

void CDlgBulge::OnTimer(UINT_PTR nIDEvent)
{
	g_bReduce = !!m_checkReduce.GetCheck();
	g_nCenterX = getEditNum(m_editCenterX);
	g_nCenterY = getEditNum(m_editCenterY);
	g_nRadius = m_sliderRadius.GetPos();
	g_fWeight = (float)m_sliderWeight.GetPos();

	g_nOrigX = getEditNum(m_editOrgX);
	g_nOrigY = getEditNum(m_editOrgY);
	g_nTgtX = getEditNum(m_editTgtX);
	g_nTgtY = getEditNum(m_editTgtY);
	g_nMoveRadius = (float)m_sliderMoveRadius.GetPos();
	g_nMoveCurve = (float)m_sliderMoveWeight.GetPos();

	CDialogEx::OnTimer(nIDEvent);
}
