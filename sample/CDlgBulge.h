#pragma once

// CDlgBulge 对话框

class CDlgBulge : public CDialogEx {
	DECLARE_DYNAMIC(CDlgBulge)

public:
	CDlgBulge(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~CDlgBulge();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BULGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editCenterX;
	CEdit m_editCenterY;
	CEdit m_editRadius;
	CSliderCtrl m_sliderRadius;

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
