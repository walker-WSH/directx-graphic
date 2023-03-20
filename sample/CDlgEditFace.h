#pragma once
#include "RenderHelper.h"
#include <vector>
// CDlgEditFace 对话框

class CDlgEditFace : public CDialogEx {
	DECLARE_DYNAMIC(CDlgEditFace)

public:
	CDlgEditFace(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~CDlgEditFace();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	struct FinishedEditor {
		CPoint m_ptOrigin;
		CPoint m_ptTarget;
		float g_nMoveRadius;
		float g_nMoveCurve;
	};

	bool m_bResized = false;
	bool m_bEditing = false;
	CButton m_btnPos;
	std::vector<FinishedEditor> m_finishedList;
	CPoint m_ptOrigin;
	CPoint m_ptTarget;

	texture_handle edit_bulgeTex = nullptr; // 窗口画面 先画到这上面 再present到窗口
	texture_handle edit_copyTex = nullptr;  // 窗口画面 先画到这上面 再present到窗口
	display_handle edit_display = nullptr;

	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持
	afx_msg LRESULT OnNcHitTest(CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
