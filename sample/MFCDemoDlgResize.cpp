
#include "pch.h"
#include "framework.h"
#include "MFCDemo.h"
#include "MFCDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
