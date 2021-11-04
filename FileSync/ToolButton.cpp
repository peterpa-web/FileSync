// ToolButton.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "ToolButton.h"


// CToolButton

IMPLEMENT_DYNAMIC(CToolButton, CBitmapButton)

CToolButton::CToolButton()
{
	m_bMouseCaptured = FALSE;
	m_nIDBitmapResource = 0U;
	m_nIDBitmapResourceDisabled = 0U;
}

CToolButton::~CToolButton()
{
}


BEGIN_MESSAGE_MAP(CToolButton, CBitmapButton)
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


// CToolButton message handlers

BOOL CToolButton::Create(CWnd* pParentWnd, UINT nID, UINT nIDBitmapResource, UINT nIDBitmapResourceDisabled)
{
	CRect rectB(0,0,20,20);
	if ( ! CBitmapButton::Create( _T(""), WS_VISIBLE | 
					BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW, rectB, pParentWnd, nID ) )
			return FALSE;

	return LoadBitmapsEx( nIDBitmapResource, nIDBitmapResourceDisabled );
}

BOOL CToolButton::LoadBitmapsEx(UINT nIDBitmapResource, UINT nIDBitmapResourceDisabled)
{
	if ( nIDBitmapResourceDisabled != 0U )
		m_nIDBitmapResourceDisabled = nIDBitmapResourceDisabled;
	m_nIDBitmapResource = nIDBitmapResource;
	if ( nIDBitmapResource == 0U )
		nIDBitmapResource = m_nIDBitmapResourceDisabled;

	if ( ! LoadBitmaps( nIDBitmapResource, 0U, 0U, nIDBitmapResourceDisabled ) )
			return FALSE;

	if ( nIDBitmapResource != 0U ) {
		m_bmTr.DeleteObject();
		m_bmTr.LoadBitmap(nIDBitmapResource);
	}
	if ( nIDBitmapResourceDisabled != 0U ) {
		m_bmTrDis.DeleteObject();
		m_bmTrDis.LoadBitmap(nIDBitmapResourceDisabled);
	}
	EnableWindow( m_nIDBitmapResource != 0U );
	return TRUE;
}

void CToolButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (lpDrawItemStruct->itemState & ODS_DISABLED) {
		m_bmTrDis.DrawTransparent(pDC, 0, 0);
		return;
	}
	m_bmTr.DrawTransparent(pDC, 0, 0);

   	// add edges
   	if (lpDrawItemStruct->itemState & ODS_SELECTED)
      	::DrawEdge(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, BDR_SUNKENINNER, BF_RECT);
 //	else if (lpDrawItemStruct->itemState & ODS_FOCUS || m_bMouseCaptured)
   	else if (m_bMouseCaptured)
      	::DrawEdge(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, BDR_RAISEDINNER, BF_RECT);
}

void CToolButton::OnMouseMove(UINT nFlags, CPoint point)
{
    if (point != m_LastMouseMovePoint)
    {
        m_LastMouseMovePoint = point;

        CRect clientRect;
        GetClientRect(clientRect);
        if (clientRect.PtInRect(point))
        {
	        // Make sure we capture mouse so we can detect when to turn off 
	        if (!m_bMouseCaptured && GetCapture() != this)
	        {
			    SetCapture();
			    m_bMouseCaptured = TRUE;
				Invalidate();
	        }
        }
		else
		{
	        if (m_bMouseCaptured)
	        {
	            VERIFY(ReleaseCapture());
	            m_bMouseCaptured = FALSE;
				Invalidate();
	        }
		}
    }

	CBitmapButton::OnMouseMove(nFlags, point);
}


void CToolButton::OnKillFocus(CWnd* pNewWnd)
{
	CBitmapButton::OnKillFocus(pNewWnd);

    if (m_bMouseCaptured)
    {
        VERIFY(ReleaseCapture());
        m_bMouseCaptured = FALSE;
		Invalidate();
    }
}
