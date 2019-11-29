#include "stdafx.h"
#include "ROEdit.h"

IMPLEMENT_DYNAMIC(CROEdit, CEdit)

CROEdit::CROEdit()
{
}

CROEdit::~CROEdit()
{
}

BEGIN_MESSAGE_MAP(CROEdit, CEdit)
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CROEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	GetParent()->SendMessage(WM_END_SELECT_TEXT, 8, 9);

	CEdit::OnLButtonUp(nFlags, point);
}

HBRUSH CROEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	// TODO:  �ڴ˸��� DC ���κ�����
	if (nCtlColor != CTLCOLOR_EDIT) {
		COLORREF clr = RGB(0, 0, 0);
		pDC->SetTextColor(clr);
		clr = RGB(255, 255, 255);
		pDC->SetBkColor(clr);
		HBRUSH br = ::CreateSolidBrush(clr);
		return br;
	}
	// TODO:  �����Ӧ���ø����Ĵ�������򷵻ط� null ����
	return NULL;
}

void CROEdit::OnKillFocus(CWnd* pNewWnd)
{
	//CEdit::OnKillFocus(pNewWnd);

	// TODO: �ڴ˴������Ϣ����������
}

BOOL CROEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYUP) {
		//BOOL bCtrl = ::GetKeyState(VK_SHIFT) & 0x80;
		WPARAM  k = pMsg->wParam;
		BOOL b = (k == VK_HOME) ||
			(k == VK_END) ||
			(k == VK_LEFT) ||
			(k == VK_RIGHT) ||
			(k == VK_UP) ||
			(k == VK_DOWN);

		if (b) {
			GetParent()->SendMessage(WM_END_SELECT_TEXT, 8, 9);
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}