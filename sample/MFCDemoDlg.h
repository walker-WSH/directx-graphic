
// MFCDemoDlg.h: 头文件
//

#pragma once

#include <Windows.h>
#include <process.h>
#include <functional>
#include "AsyncTask.h"
#include "IGraphicDefine.h"
#include "IGraphicEngine.h"

using namespace graphic;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define DRAGE_REGION_SIZE 40
#define RESIZE_REGION_SIZE 10

// CMFCDemoDlg 对话框
class CMFCDemoDlg : public CDialogEx {
	// 构造
public:
	CMFCDemoDlg(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~CMFCDemoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCDEMO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;
	CButton m_btnExit;
	CButton m_checkboxSmooth;
	CSliderCtrl m_lineSizeSlider;
	CSliderCtrl m_gausBlurSlider;
	CSliderCtrl m_rotateSlider;

	bool m_bToDrawCurve = false;
	bool m_bIsDrawing = false;
	std::vector<CPoint> m_vCurrentPoints;

	AsyncTask m_asyncTask;

	bool m_bExit = false;
	int m_nResizeState = 0;
	bool m_bSaveImage = false;
	HANDLE m_hThread = 0;
	IGraphicSession *m_pGraphic = nullptr;
	static unsigned __stdcall ThreadFuncNormalRender(void *pParam);
	static unsigned __stdcall ThreadFuncForSubRegionMosic(void *pParam);

public:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedDrawCurve();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedClearFullBk();
	afx_msg void OnBnClickedClearAllDraw();
	afx_msg void OnNMThemeChangedSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton10();
};
