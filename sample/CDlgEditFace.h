#pragma once


// CDlgEditFace 对话框

class CDlgEditFace : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEditFace)

public:
	CDlgEditFace(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgEditFace();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	bool m_bResized = false;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
