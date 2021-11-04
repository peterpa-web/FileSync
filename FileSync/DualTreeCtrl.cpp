// DualTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
//#include "ViewDir.h"
#include "DualTreeItem.h"

#include "DualTreeCtrl.h"


// CDualTreeCtrl

IMPLEMENT_DYNAMIC(CDualTreeCtrl, CTreeCtrl)
CDualTreeCtrl::CDualTreeCtrl()
{
	m_pFont = NULL;
	m_nItemHeight = 10;
	m_nCharWidth = 5;
//	m_pView = NULL;
//	m_nPageSize = 0;
	m_nCharsLeft = 0;
	m_nCharsRight = 0;
	m_nOffsLeft = 0;
	m_nOffsRight = 200;
	m_bEnableDraw = TRUE;
	m_bEnableClick = TRUE;
	m_pMenuContext = NULL;
	m_nSide = CDualTreeItem::left;
}

CDualTreeCtrl::~CDualTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CDualTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
//	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnLbnSelchange)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
//	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
//	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnTvnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnTvnItemexpanded)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CDualListBox message handlers


void CDualTreeCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
	if ( lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
//		TRACE1( "Prepaint %d\n",lplvcd->nmcd.dwItemSpec);
		MEASUREITEMSTRUCT measureItemStruct;
		MeasureItem( &measureItemStruct );
		return;
	}
	else if ( lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		DrawItem( lplvcd );

		*pResult = CDRF_NEWFONT; //CDRF_SKIPDEFAULT;	// CDRF_DODEFAULT; CDRF_NEWFONT
		return;
	}
	*pResult = 0;
}

void CDualTreeCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
//	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	if ( m_pFont != NULL )
		SetFont( m_pFont );
	m_pFont = NULL;

	if ( m_nCharWidth == 5 ) {		// init
		CSize   sz;
		CDC*    pDC = GetDC();
		sz = pDC->GetTextExtent(_T("Tg"));
		ReleaseDC(pDC);
		m_nItemHeight = sz.cy;
		m_nCharWidth = (sz.cx+2) / 2;
		TRACE1("CDualTreeCtrl::MeasureItem cw=%d\n", m_nCharWidth);
		UpdOffs();
	}
	lpMeasureItemStruct->itemHeight = m_nItemHeight;
}


void CDualTreeCtrl::DrawItem(LPNMTVCUSTOMDRAW lplvcd)
{
//	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
//	TRACE1( "CDualTreeCtrl::DrawItem %d\n",lplvcd->nmcd.dwItemSpec);

	if ( !m_bEnableDraw )
		return;

	CDC dc;
	dc.Attach(lplvcd->nmcd.hdc);

	// Save these value to restore them when done drawing.
	int nOldBkMode = dc.SetBkMode( TRANSPARENT );
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

//	ASSERT( m_pView != NULL );
	DrawTreeItem( &dc, lplvcd );

	// Reset the background color and the text color back to their
	// original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);
	dc.SetBkMode( nOldBkMode );

	dc.Detach();
}


BOOL CDualTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));
    return TRUE;

//	return CListBox::OnEraseBkgnd(pDC);
}

void CDualTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect( rect );
	CRect rectLeft = rect;
	rectLeft.left = m_nOffsLeft;
	rectLeft.right = m_nOffsRight;
	CRect rectRight = rect;
	rectRight.left = m_nOffsRight;

	if ( rectLeft.PtInRect( point ) )
	{
		if ( m_nSide != CDualTreeItem::left )
		{
			m_nSide = CDualTreeItem::left;
			m_nClickSide = CDualTreeItem::left;
			Invalidate();
		}
	}
	else if ( rectRight.PtInRect( point ) )
	{
		if ( m_nSide != CDualTreeItem::right )
		{
			m_nSide = CDualTreeItem::right;
			m_nClickSide = CDualTreeItem::right;
			Invalidate();
		}
	}

	if ( !m_bEnableClick )
		return;

	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	if ((hItem != NULL) && ((TVHT_ONITEM | TVHT_ONITEMRIGHT) & uFlags))
	{
		m_nMouseKeyFlags = nFlags;
		Select(hItem, TVGN_CARET);
	}
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CDualTreeCtrl::UpdOffs()
{
	CRect rect;
	GetClientRect( rect );
	int nOffsLeft;
	int nOffsRight;
//	m_nPageSize = rect.Height() / (m_nItemHeight+2);
//	if ( GetVisibleCount() < m_nPageSize )					// 20100819
//		rect.right -= 20;	// reserve for scroll bar
	if ( m_nCharsLeft == 0 )
	{
		nOffsLeft = 0;
		nOffsRight = (rect.right+1) / 2;
	}
	else
	{
		//int nMaxRight = rect.right;
		nOffsRight = rect.Width() - m_nCharsRight * m_nCharWidth;
		nOffsLeft  = nOffsRight - m_nCharsLeft * m_nCharWidth;
	}
	TRACE("CDualTreeCtrl::UpdOffs() cw=%d,w=%d,ol=%d,or=%d\n", m_nCharWidth, rect.Width(), m_nOffsLeft, m_nOffsRight );

	if ( m_nOffsLeft != nOffsLeft )
		Invalidate();

	m_nOffsRight = nOffsRight;
	m_nOffsLeft = nOffsLeft;
}

void CDualTreeCtrl::UpdCharsLeft( int nChars )
{
	if (nChars != m_nCharsLeft) {
		TRACE1("CDualTreeCtrl::UpdCharsLeft() %d\n", nChars);
		m_nCharsLeft = nChars; 
		UpdOffs(); 
		Invalidate();
	} 
}

void CDualTreeCtrl::UpdCharsRight( int nChars )
{ 
	if (nChars != m_nCharsRight) {
		TRACE1("CDualTreeCtrl::UpdCharsRight() %d\n", nChars);
		m_nCharsRight = nChars; 
		UpdOffs(); 
		Invalidate();
	} 
}

void CDualTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	AdjustVScroll();
	CTreeCtrl::OnSize(nType, cx, cy);
	TRACE2("CDualTreeCtrl::OnSize %d %d\n", cx, cy);
	UpdOffs();
}

HTREEITEM CDualTreeCtrl::GetBottomItem(void)
{
//	int i = m_nPageSize + GetTopIndex() - 1;
	HTREEITEM hItem = GetFirstVisibleItem();
	UINT n = GetVisibleCount();
	if ( n > 0 )
		--n;
	while ( hItem != NULL && n != 0 )
	{
		hItem = GetNextVisibleItem( hItem );
		--n;
	}
	return hItem;
}

BOOL CDualTreeCtrl::IsItemVisible( HTREEITEM hItem )
{
	HTREEITEM hItemV = GetFirstVisibleItem();
	UINT n = GetVisibleCount();
	if ( n > 0 )
		--n;
	while ( hItemV != NULL && n != 0 )
	{
		if ( hItem == hItemV )
			return TRUE;
		hItemV = GetNextVisibleItem( hItemV );
		--n;
	}
	return FALSE;
}

int CDualTreeCtrl::GetViewLineLen( int nResPixels ) const
{
	if ( m_hWnd == NULL )
		return 0;
	CRect rect;
	GetClientRect( rect );
	return (rect.Width() - nResPixels) / m_nCharWidth;
}



void CDualTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// todo: HitTest and save hItem & set m_nSide
	// use NM_DBLCLK on parent window

	if ( !m_bEnableClick )
		return;

	HTREEITEM hItem = GetSelectedItem();

	int nLevel = 0;
	HTREEITEM hParentItem = GetParentItem( hItem );
	while ( hParentItem != NULL )
	{
		++nLevel;
		hParentItem = GetParentItem( hParentItem );
	}
	int left = 18 + nLevel * GetIndent();

	m_hItemCurr = hItem;
	if ( point.x <= left )
		return;
	if ( point.x < m_nOffsLeft )
		m_nClickSide = CDualTreeItem::common;
	else if ( point.x < m_nOffsRight )
		m_nClickSide = CDualTreeItem::left;
	else
		m_nClickSide = CDualTreeItem::right;

	NMHDR notify;
	notify.code = NM_DBLCLK;
	notify.hwndFrom = m_hWnd;
	notify.idFrom = GetDlgCtrlID();
	GetParent()->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&notify );
}

void CDualTreeCtrl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CDualTreeCtrl::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	OnTvnGetdispinfo( pTVDispInfo ); 
	*pResult = 0;
}

void CDualTreeCtrl::OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
	AdjustVScroll(); 
	*pResult = 0;
}

void CDualTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest( point, &uFlags );
	if ( hItem != NULL && m_pMenuContext != NULL && ((TVHT_ONITEM | TVHT_ONITEMRIGHT) & uFlags) )
	{
		if ( point.x >= m_nOffsLeft && point.x < m_nOffsRight )
			m_nClickSide = CDualTreeItem::left;
		else if ( point.x >= m_nOffsRight )
			m_nClickSide = CDualTreeItem::right;
		else
			m_nClickSide = CDualTreeItem::common;

		if ( m_nClickSide < CDualTreeItem::common )
			m_nSide = m_nClickSide;

		m_hItemCurr = hItem;
		POINT pt = point;
		ClientToScreen( &pt );
		ShowPopup( &pt );
		return;
	}
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CDualTreeCtrl::InvalidateItem( HTREEITEM hItem )
{
	RECT r;
	if ( GetItemRect(hItem, &r, FALSE) )
		InvalidateRect( &r );
}

void CDualTreeCtrl::AdjustVScroll()
{
	SCROLLINFO info;
	info.nPage = 0;
	if ( GetScrollInfo( SB_VERT, &info ) ) {
		TRACE2( "CDualTreeCtrl::AdjustVScroll %d %d\n", info.nPage, info.nMax );
		if ( info.nPage == 0 || (int)info.nPage >= info.nMax ) {
			ShowScrollBar( SB_VERT );
			EnableScrollBar( SB_VERT, FALSE );
		}
	}
}
