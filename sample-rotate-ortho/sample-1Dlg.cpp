
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
	CoInitialize(nullptr);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

Csample1Dlg::~Csample1Dlg()
{
	CoUninitialize();
}

void Csample1Dlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_X, m_edit_x);
	DDX_Control(pDX, IDC_EDIT_Y, m_edit_y);
}

BEGIN_MESSAGE_MAP(Csample1Dlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_DESTROY()
ON_WM_TIMER()
ON_WM_ERASEBKGND()
ON_BN_CLICKED(IDC_BUTTON_CLEAR, &Csample1Dlg::OnBnClickedButtonClear)
ON_BN_CLICKED(IDC_BUTTON_ADD_MOVE, &Csample1Dlg::OnBnClickedButtonAddMove)
ON_BN_CLICKED(IDC_BUTTON_ADD_SCALE, &Csample1Dlg::OnBnClickedButtonAddScale)
ON_WM_LBUTTONDOWN()
ON_BN_CLICKED(IDC_BUTTON_ROTATE, &Csample1Dlg::OnBnClickedButtonRotate)
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

void Csample1Dlg::OnBnClickedButtonClear()
{
	m_worldList.clear();
}

float GetFloatValue(CEdit *pEdit)
{
	CString strInput;
	pEdit->GetWindowText(strInput);

	if (strInput.IsEmpty())
		return 0.f;

	// 将文本转换为浮点数
	double fValue = _ttof(strInput);
	return (float)fValue;
}

void Csample1Dlg::OnBnClickedButtonRotate()
{
	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_ROTATE;
	vec.z = 45;

	m_worldList.push_back(vec);
}

void Csample1Dlg::OnBnClickedButtonAddMove()
{
	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_MOVE;
	vec.x = GetFloatValue(&m_edit_x);
	vec.y = GetFloatValue(&m_edit_y);

	m_worldList.push_back(vec);
}

void Csample1Dlg::OnBnClickedButtonAddScale()
{
	// 左上角为起点时  scale时就不是以图像当前move的位置的左上角为起点了
	// 即：如果图像先move再scale，则scale会引起图像左上角的位置
	WorldVector vec;
	vec.type = WORLD_TYPE::VECTOR_SCALE;

	auto x = GetFloatValue(&m_edit_x);
	if (x > 0.1f || x < -0.1f)
		vec.x = x;

	auto y = GetFloatValue(&m_edit_y);
	if (y > 0.1f || y < -0.1f)
		vec.y = y;

	m_worldList.push_back(vec);
}

bool Csample1Dlg::IsPointOnImage(const CPoint &pt) // 判断屏幕上某个点 是否命中了图片
{
	XMVECTOR ptTemp = XMVectorSet((float)pt.x, (float)pt.y, 0.0f, 1.0f);
	XMMATRIX inverseWorldMatrix = XMMatrixInverse(nullptr, m_worldMatrix);
	XMVECTOR pointLocal = XMVector4Transform(ptTemp, inverseWorldMatrix);

	auto x = pointLocal.m128_f32[0];
	auto y = pointLocal.m128_f32[1];

	auto is_selected = (x > 0 && x < m_texMainImgInfo.width && y > 0 && y < m_texMainImgInfo.height);
	ATLTRACE("----------- selected=%d (%d, %d) image=%dx%d \n", is_selected, (int)x, (int)y, m_texMainImgInfo.width, m_texMainImgInfo.height);
	return is_selected;
}

void Csample1Dlg::GetTextureScreenPos(CPoint &lt, CPoint &rb) // 获取图标在窗口上渲染的位置
{
		XMVECTOR lt_pt = XMVectorSet(0.f, 0.f, 0.0f, 1.0f);
		XMVECTOR screenLeftTop = XMVector4Transform(lt_pt, m_worldMatrix);
		lt.x = (LONG)screenLeftTop.m128_f32[0];
		lt.y = (LONG)screenLeftTop.m128_f32[1];

		XMVECTOR rb_pt = XMVectorSet((float)m_texMainImgInfo.width, (float)m_texMainImgInfo.height, 0.0f, 1.0f);
		XMVECTOR rightBottom = XMVector4Transform(rb_pt, m_worldMatrix);
		rb.x = (LONG)rightBottom.m128_f32[0];
		rb.y = (LONG)rightBottom.m128_f32[1];
}

void Csample1Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_selected = IsPointOnImage(point);
}
