
// MFCDemoDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCDemo.h"
#include "MFCDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

float g_lineStride = 2.f;

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

// CMFCDemoDlg 对话框

CMFCDemoDlg::CMFCDemoDlg(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_MFCDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pGraphic = graphic::CreateGraphicSession();
}

CMFCDemoDlg::~CMFCDemoDlg()
{
	m_bExit = true;
	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	graphic::DestroyGraphicSession(m_pGraphic);
}

void CMFCDemoDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_btnTest);
	DDX_Control(pDX, IDC_CHECK_SMOOTH, m_checkboxSmooth);
	DDX_Control(pDX, IDC_SLIDER1, m_lineSizeSlider);
}

BEGIN_MESSAGE_MAP(CMFCDemoDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_LBUTTONDOWN()
ON_WM_NCHITTEST()
ON_WM_NCLBUTTONDOWN()
ON_WM_SETCURSOR()
ON_WM_LBUTTONDBLCLK()
ON_WM_MBUTTONDOWN()
ON_WM_RBUTTONDOWN()
ON_BN_CLICKED(IDC_BUTTON1, &CMFCDemoDlg::OnBnClickedButton1)
ON_WM_ERASEBKGND()
ON_WM_ENTERSIZEMOVE()
ON_WM_EXITSIZEMOVE()
ON_WM_SIZE()
ON_WM_SIZING()
ON_BN_CLICKED(IDC_BUTTON2, &CMFCDemoDlg::OnBnClickedButton2)
ON_BN_CLICKED(IDC_BUTTON3, &CMFCDemoDlg::OnBnClickedButton3)
ON_BN_CLICKED(IDC_BUTTON4, &CMFCDemoDlg::OnBnClickedButton4)
ON_BN_CLICKED(IDC_BUTTON5, &CMFCDemoDlg::OnBnClickedButton5)
ON_BN_CLICKED(IDC_BUTTON6, &CMFCDemoDlg::OnBnClickedButton6)
ON_BN_CLICKED(IDC_BUTTON7, &CMFCDemoDlg::OnBnClickedDrawCurve)
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_BN_CLICKED(IDC_BUTTON8, &CMFCDemoDlg::OnBnClickedClearFullBk)
ON_BN_CLICKED(IDC_BUTTON9, &CMFCDemoDlg::OnBnClickedClearAllDraw)
ON_NOTIFY(NM_THEMECHANGED, IDC_SLIDER1, &CMFCDemoDlg::OnNMThemeChangedSlider1)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BUTTON10, &CMFCDemoDlg::OnBnClickedButton10)
END_MESSAGE_MAP()

// CMFCDemoDlg 消息处理程序

BOOL CMFCDemoDlg::OnInitDialog()
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

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	MoveWindow(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	CenterWindow();

	m_lineSizeSlider.SetRange(1, 20);
	m_lineSizeSlider.SetPos((int)g_lineStride);

	m_bExit = false;
	m_hThread = (HANDLE)_beginthreadex(0, 0, ThreadFunc, this, 0, 0);

	SetTimer(3000, 50, nullptr);
	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMFCDemoDlg::OnNcHitTest(CPoint pt)
{
	RECT rcWindow;
	::GetWindowRect(m_hWnd, &rcWindow);

	// 最好将四个角的判断放在前面
	if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE && pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE && pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE && pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE && pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE)
		return HTLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE)
		return HTRIGHT;
	else if (pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOP;
	else if (pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOM;

	if (pt.y <= (rcWindow.top + DRAGE_REGION_SIZE))
		return HTCAPTION;
	else
		return __super::OnNcHitTest(pt);
}

void CMFCDemoDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTTOP:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOM:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, MAKELPARAM(point.x, point.y));
		break;
	case HTLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, MAKELPARAM(point.x, point.y));
		break;
	default:
		__super::OnNcLButtonDown(nHitTest, point);
	}
}

BOOL CMFCDemoDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	switch (nHitTest) {
	case HTTOP:
	case HTBOTTOM:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
		return TRUE;

	case HTLEFT:
	case HTRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return TRUE;

	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
		return TRUE;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW)));
		return TRUE;

	default:
		return __super::OnSetCursor(pWnd, nHitTest, message);
	}
}

void CMFCDemoDlg::OnBnClickedButton1()
{
	m_btnTest.ShowWindow(SW_HIDE);
}

BOOL CMFCDemoDlg::OnEraseBkgnd(CDC *pDC)
{
	return TRUE; // 不擦除背景
}

void CMFCDemoDlg::OnPaint()
{
	if (m_nResizeState > 1) {
		// resizing期间 DX停止渲染 需要用GDI渲染
		CRect rect;
		GetClientRect(&rect);

		CBrush brush(RGB(0, 0, 255));

		CPaintDC dc(this);
		dc.FillRect(&rect, &brush);

	} else {
		// 如果OnEraseBkgnd返回了TRUE, 暂停DX渲染后只执行这句，界面是黑色的
		CDialogEx::OnPaint();
	}
}

void CMFCDemoDlg::OnEnterSizeMove()
{
	m_nResizeState = 1; // 开始resize，DX暂停绘制
	CDialogEx::OnEnterSizeMove();
}

void CMFCDemoDlg::OnExitSizeMove()
{
	m_nResizeState = 0; // 结束resize，DX恢复绘制
	CDialogEx::OnExitSizeMove();
}

void CMFCDemoDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	if (m_nResizeState == 1)
		m_nResizeState += 1;
	if (m_nResizeState > 1)
		RedrawWindow(); // 立即触发一次重绘
}

void CMFCDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (m_nResizeState == 1)
		m_nResizeState += 1;
	if (m_nResizeState > 1)
		RedrawWindow(); // 立即触发一次重绘

	if (m_btnTest.GetSafeHwnd()) {
		const int width = 400;
		const int height = 50;
		int left = (cx - width) / 2;
		int top = cy - height - 10;
		m_btnTest.MoveWindow(left, top, width, height);
	}
}

int gridX = 3;
int gridY = 3;

void CMFCDemoDlg::OnBnClickedButton2()
{
	DestroyWindow();
}

void CMFCDemoDlg::OnBnClickedButton3()
{
	++gridX;
}

void CMFCDemoDlg::OnBnClickedButton4()
{
	if (gridX <= 1)
		return;

	--gridX;
}

void CMFCDemoDlg::OnBnClickedButton5()
{
	++gridY;
}

void CMFCDemoDlg::OnBnClickedButton6()
{
	if (gridY <= 1)
		return;

	--gridY;
}

void CMFCDemoDlg::OnNMThemeChangedSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	*pResult = 0;
}

void CMFCDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	g_lineStride = (float)m_lineSizeSlider.GetPos();

	CDialogEx::OnTimer(nIDEvent);
}

void CMFCDemoDlg::OnBnClickedButton10()
{
	m_bSaveImage = true;
}
