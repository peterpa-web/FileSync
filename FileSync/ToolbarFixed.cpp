// ToolbarFixed.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "ToolbarFixed.h"


// CToolbarFixed

IMPLEMENT_DYNAMIC(CToolBarFixed, CMFCToolBar)
CToolBarFixed::CToolBarFixed()
{
}

CToolBarFixed::~CToolBarFixed()
{
}


BEGIN_MESSAGE_MAP(CToolBarFixed, CMFCToolBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CToolbarFixed-Meldungshandler


void CToolBarFixed::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	CMFCToolBar::OnLButtonDown(nFlags, point);
}

void CToolBarFixed::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	CMFCToolBar::OnLButtonDblClk(nFlags, point);
}

void CToolBarFixed::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	// CToolBar::OnRButtonDown(nFlags, point);
}
