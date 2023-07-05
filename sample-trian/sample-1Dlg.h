
// sample-1Dlg.h: 头文件
//

#pragma once

#include "lib-graphic/IGraphicEngine.h"
using namespace graphic;

#define TIMER_RENDER 3000

// Csample1Dlg 对话框
class Csample1Dlg : public CDialogEx
{
// 构造
public:
	Csample1Dlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~Csample1Dlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMPLE1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void initGraphic(HWND hWnd);
	void uninitGraphic();
	void render();

	void RenderTexture(SIZE canvas, RECT drawDest);
	float getRotate();

// 实现
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
	CSliderCtrl m_sliderRotate;
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
};
