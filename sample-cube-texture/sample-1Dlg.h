
// sample-1Dlg.h: 头文件
//

#pragma once

#include "lib-graphic/IGraphicEngine.h"
using namespace graphic;

#define TIMER_RENDER 3000

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT WINDOW_WIDTH

// Csample1Dlg 对话框
class Csample1Dlg : public CDialogEx {
	// 构造
public:
	Csample1Dlg(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~Csample1Dlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMPLE1_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	void initGraphic(HWND hWnd);
	void uninitGraphic();
	void render();

	void RenderTexture(texture_handle tex, SIZE canvas, RECT drawDest);

	int eyePosZ = 0; // [-100, 100]

	std::vector<WorldVector> worldList;
	bool m_bLBDown = false;
	CPoint m_ptDown;
	CPoint m_ptMove;

	WorldVector get_rotate();

protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
