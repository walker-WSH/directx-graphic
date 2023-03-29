#pragma once

#include "RenderHelper.h"
#include <vector>

// CDlgHighlight 对话框

class CDlgHighlight : public CDialogEx {
	DECLARE_DYNAMIC(CDlgHighlight)

public:
	CDlgHighlight(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~CDlgHighlight();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_HIGHLIGHT };
#endif

	bool test_highlight = false;

protected:
	texture_handle edit_canvasTex = nullptr; // 窗口画面 先画到这上面 再present到窗口
	display_handle edit_display = nullptr;

	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	float highlight = 0.f;
	float shadow = 0.f;
	float clarity = 0.f;
	float redius = 1.25f;

	CSliderCtrl m_sliderCLARITY;
	CSliderCtrl m_sliderSHADOWS;
	CSliderCtrl m_sliderHIGHLIGHTS;
	CSliderCtrl m_sliderRADIUS;
};
