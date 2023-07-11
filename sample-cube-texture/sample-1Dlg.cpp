
// sample-1Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "sample-1.h"
#include "sample-1Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// Csample1Dlg 对话框

Csample1Dlg::Csample1Dlg(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_SAMPLE1_DIALOG, pParent)
{
	CoInitialize(NULL);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

Csample1Dlg::~Csample1Dlg()
{
	CoUninitialize();
}

void Csample1Dlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Csample1Dlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_DESTROY()
ON_WM_TIMER()
ON_WM_ERASEBKGND()
ON_BN_CLICKED(IDC_BUTTON7, &Csample1Dlg::OnBnClickedButton7)
ON_BN_CLICKED(IDC_BUTTON5, &Csample1Dlg::OnBnClickedButton5)
ON_BN_CLICKED(IDC_BUTTON6, &Csample1Dlg::OnBnClickedButton6)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// Csample1Dlg 消息处理程序

BOOL Csample1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);  // 设置大图标
	SetIcon(m_hIcon, FALSE); // 设置小图标

	// TODO: 在此添加额外的初始化代码
	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	MoveWindow(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	CenterWindow();

	initGraphic(m_hWnd);
	SetTimer(TIMER_RENDER, 33, 0);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void Csample1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Csample1Dlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Csample1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Csample1Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	uninitGraphic();
}

void Csample1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == TIMER_RENDER) {
		render();
	}

	CDialogEx::OnTimer(nIDEvent);
}

BOOL Csample1Dlg::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void Csample1Dlg::OnBnClickedButton5()
{
	eyePosZ += 10;
}

void Csample1Dlg::OnBnClickedButton6()
{
	eyePosZ -= 10;
}

void Csample1Dlg::OnBnClickedButton7()
{
	worldList.clear();
	eyePosZ = 0;
}

WorldVector Csample1Dlg::get_rotate()
{
	assert(m_bLBDown);

	CPoint delta = m_ptMove - m_ptDown;

	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_ROTATE;
	vec.x = 100.f * float(delta.y) / 500.f;
	vec.y = 100.f * float(delta.x) / 500.f;

	return vec;
}

void Csample1Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBDown = true;
	m_ptDown = m_ptMove = point;
	SetCapture();
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Csample1Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	worldList.push_back(get_rotate());
	m_bLBDown = false;

	ReleaseCapture();
	CDialogEx::OnLButtonUp(nFlags, point);
}

void Csample1Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLBDown) {
		m_ptMove = point;
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL Csample1Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	auto step = 5;
	if (zDelta > 0) {
		eyePosZ += step;
	} else {
		eyePosZ -= step;
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}
