#pragma once
#include "BitmapTransparent.h"


// CToolButton

class CToolButton : public CBitmapButton
{
	DECLARE_DYNAMIC(CToolButton)

public:
	CToolButton();
	virtual ~CToolButton();
	BOOL Create(CWnd* pParentWnd, UINT nID, UINT nIDBitmapResource, UINT nIDBitmapResourceDisabled = 0U);
	BOOL LoadBitmapsEx(UINT nIDBitmapResource, UINT nIDBitmapResourceDisabled = 0U);
	UINT GetBitmapNo() { return (IsWindowEnabled() ? m_nIDBitmapResource : 0U); }

protected:
	DECLARE_MESSAGE_MAP()
	CBitmapTransparent m_bmTr;
	CBitmapTransparent m_bmTrDis;
    CPoint m_LastMouseMovePoint; // Last position of mouse cursor
    BOOL m_bMouseCaptured; // Is mouse captured?
	UINT m_nIDBitmapResource;
	UINT m_nIDBitmapResourceDisabled;

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};


