
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

float g_lineStride = 100.f;
float g_blurValue = 50.f;
int g_rotatePeriod = 10 * 1000;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

static const std::wstring TEST_MOSAIC_CMD = L"test_mosaic";
enum class RUN_TEST_FOR {
	RUN_NORMAL = 0,
	RUN_SUB_MOSAIC,
};
RUN_TEST_FOR InitCmdParams()
{
	RUN_TEST_FOR ret = RUN_TEST_FOR::RUN_NORMAL;
	LPWSTR *szArgList = NULL;

	do {
		int argCount = 0;
		szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
		if (!szArgList) {
			break;
		}

		for (int i = 0; i < argCount; i++) {
			auto str = szArgList[i];
			if (str == TEST_MOSAIC_CMD) {
				ret = RUN_TEST_FOR::RUN_SUB_MOSAIC;
				break;
			}
		}

	} while (0);

	if (szArgList)
		LocalFree(szArgList);

	return ret;
}

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
	DDX_Control(pDX, IDC_CHECK_SMOOTH, m_checkboxSmooth);
	DDX_Control(pDX, IDC_SLIDER1, m_lineSizeSlider);
	DDX_Control(pDX, IDC_SLIDER2, m_gausBlurSlider);
	DDX_Control(pDX, IDC_SLIDER3, m_rotateSlider);
	DDX_Control(pDX, IDC_BUTTON2, m_btnExit);
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
ON_BN_CLICKED(IDC_BUTTON_TEST_MOSAIC, &CMFCDemoDlg::OnBnClickedButtonTestMosaic)
ON_BN_CLICKED(IDC_BUTTON_RUN_NORMAL, &CMFCDemoDlg::OnBnClickedButtonRunNormal)
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

	m_lineSizeSlider.SetRange(1, 200);
	m_lineSizeSlider.SetPos((int)g_lineStride);

	m_gausBlurSlider.SetRange(0, 100);
	m_gausBlurSlider.SetPos((int)g_blurValue);

	m_rotateSlider.SetRange(1 * 1000, 20 * 1000);
	m_rotateSlider.SetPos(g_rotatePeriod);

	m_bExit = false;
	switch (InitCmdParams()) {
	case RUN_TEST_FOR::RUN_SUB_MOSAIC:
		m_hThread = (HANDLE)_beginthreadex(0, 0, ThreadFuncForSubRegionMosic, this, 0, 0);
		break;
	case RUN_TEST_FOR::RUN_NORMAL:
	default:
		m_hThread = (HANDLE)_beginthreadex(0, 0, ThreadFuncNormalRender, this, 0, 0);
		break;
	}

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

	if (m_btnExit.GetSafeHwnd()) {
		const auto exitBtnSize = 50;
		const auto border = 10;
		m_btnExit.MoveWindow(cx - exitBtnSize - border, border, exitBtnSize, exitBtnSize);
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
	g_blurValue = (float)m_gausBlurSlider.GetPos();
	g_rotatePeriod = m_rotateSlider.GetPos();

	CDialogEx::OnTimer(nIDEvent);
}

void CMFCDemoDlg::OnBnClickedButton10()
{
	m_bSaveImage = true;
}

void RunChildProcess(const wchar_t *param)
{
	wchar_t path[MAX_PATH] = {};
	GetModuleFileNameW(0, path, MAX_PATH);

	wchar_t cmd[2048];
	swprintf_s(cmd, 2048, L"\"%s\" \"%s\" ", path, param ? param : L"");

	PROCESS_INFORMATION pi = {};
	STARTUPINFO si = {};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;
	si.wShowWindow = SW_HIDE;

	BOOL bOK = CreateProcessW(path, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	if (!bOK) {
		assert(false);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void CMFCDemoDlg::OnBnClickedButtonTestMosaic()
{
	RunChildProcess(TEST_MOSAIC_CMD.c_str());
	DestroyWindow(); // exit current process
}

void CMFCDemoDlg::OnBnClickedButtonRunNormal()
{
	RunChildProcess(nullptr);
	DestroyWindow(); // exit current process
}
