// DualListBox.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "ViewList.h"

#include "DualListBox.h"


// CDualListBox

//IMPLEMENT_DYNAMIC(CDualListBox, CListBox)
IMPLEMENT_DYNAMIC(CDualListBox, CWnd)
CDualListBox::CDualListBox()
{
	m_nItems = 0;
	m_nAnchor = 0;
	m_nCaret = 0;
	m_nSelFirst = -1;
	m_nSelLast = -1;

	m_pFont = NULL;
	m_nItemHeight = 5;
	m_nCharWidth = 5;
	m_pView = NULL;
	m_nPageSize = 0;
	m_nCharsLeft = 0;
	m_nCharsRight = 0;
	m_nOffsLeft = 0;
	m_nOffsRight = 200;
	m_bEnableDraw = TRUE;
}

CDualListBox::~CDualListBox()
{
}


//BEGIN_MESSAGE_MAP(CDualListBox, CListBox)
BEGIN_MESSAGE_MAP(CDualListBox, CWnd)
//ON_CONTROL_REFLECT(LBN_SELCHANGE, OnLbnSelchange)
ON_WM_ERASEBKGND()
ON_WM_LBUTTONDOWN()
ON_WM_SIZE()
ON_WM_LBUTTONDBLCLK()
//ON_WM_RBUTTONDOWN()
ON_WM_VSCROLL()
ON_WM_PAINT()
ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()



// CDualListBox message handlers


void CDualListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_nItemHeight != 5 )
		return;

	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	if ( m_pFont != NULL )
		SetFont( m_pFont );
//	m_pFont = NULL;						// 20060315

	CSize   sz;
	CDC*    pDC = GetDC();
	sz = pDC->GetTextExtent(_T("Tg"));
	ReleaseDC(pDC);
	m_nItemHeight = sz.cy;
	m_nCharWidth = sz.cx / 2;
	lpMeasureItemStruct->itemHeight = m_nItemHeight;
}

void CDualListBox::OnLbnSelchange()
{
	int nAnchor = GetAnchorIndex();
	int nCaret = GetCaretIndex();
	int nCount = GetSelCount();
	TRACE3( "OnLbnSelchange a=%d c=%d n=%d\n", nAnchor, nCaret, nCount );
	int nSel = nAnchor < nCaret ? nCaret - nAnchor : nAnchor - nCaret;
	if ( nCount > 0 && nCount != (nSel+1) )
	{
		SetSel( -1, FALSE );
		if ( nSel == 0 )
			SetSel( nAnchor );
		else
			SelItemRange( TRUE, nAnchor, nCaret );
		nCount = nSel + 1;
	}
	// deselect last entry
	/*
	if ( nAnchor < nCaret )
		nCaret = nAnchor;
	if ( (nCaret + nCount) >= GetCount() )
	{
		--nCount;
		SetSel( -1, FALSE );
		if ( nCount == 0 )
			SetCaretIndex( nCaret );
		else if ( nCount == 1 )
			SetSel( nCaret );
		else
			SelItemRange( TRUE, nCaret, nCaret+nCount-1 );
	}
	*/
}

void CDualListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);

	if ( !m_bEnableDraw )
		return;

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	// Save these value to restore them when done drawing.
	int nOldBkMode = dc.SetBkMode( TRANSPARENT );
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	ASSERT( m_pView != NULL );
	if ( lpDrawItemStruct->itemID != LB_ERR )
		m_pView->DrawLBItem( &dc, lpDrawItemStruct );

	// Reset the background color and the text color back to their
	// original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);
	dc.SetBkMode( nOldBkMode );

	dc.Detach();
}

BOOL CDualListBox::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));
    return TRUE;

//	return CListBox::OnEraseBkgnd(pDC);
}

void CDualListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect( rect );
	CRect rectLeft = rect;
	rectLeft.left = m_nOffsLeft;
	rectLeft.right = m_nOffsRight;
	CRect rectRight = rect;
	rectRight.left = m_nOffsRight;

	CViewFileSync::Side nOldSide = m_pView->GetSide();
	if ( rectLeft.PtInRect( point ) )
	{
		if ( nOldSide != CViewFileSync::left )
		{
			m_pView->SelectSide( CViewFileSync::left );
			Invalidate();
		}
	}
	else if ( rectRight.PtInRect( point ) )
	{
		if ( nOldSide != CViewFileSync::right )
		{
			m_pView->SelectSide( CViewFileSync::right );
			Invalidate();
		}
	}
	m_pView->OnListLButtonDown();
//	CListBox::OnLButtonDown(nFlags, point);		// 200603
	BOOL bOutside;
	UINT nItem = ItemFromPoint( point, bOutside );
	if ( !bOutside )
	{
		m_nCaret = nItem;
		if ( (nFlags & MK_SHIFT) == 0 )
			SetAnchorIndex( nItem );
		if ( m_nAnchor < m_nCaret )
		{
			m_nSelFirst = m_nAnchor;
			m_nSelLast = m_nCaret;
		}
		else
		{
			m_nSelFirst = m_nCaret;
			m_nSelLast = m_nAnchor;
		}
		Invalidate();
	}
}

void CDualListBox::UpdOffs()
{
	MEASUREITEMSTRUCT mi;
	mi.CtlType = ODT_LISTBOX;
	MeasureItem( &mi );

	CRect rect;
	GetClientRect( rect );
	m_nPageSize = rect.Height() / m_nItemHeight;
	if ( m_nCharsLeft == 0 )
	{
		m_nOffsLeft = 0;
		m_nOffsRight = rect.right / 2;
	}
	else
	{
		int nMaxRight = rect.right;
		m_nOffsRight = rect.Width() - m_nCharsRight * m_nCharWidth;
		m_nOffsLeft  = m_nOffsRight - m_nCharsLeft * m_nCharWidth;
	}
	SetScrollRange( SB_VERT, 0, m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize );		// 20060315
}

void CDualListBox::OnSize(UINT nType, int cx, int cy)
{
//	CListBox::OnSize(nType, cx, cy);
	CWnd::OnSize(nType, cx, cy);
	UpdOffs();
}

int CDualListBox::GetBottomIndex(void)
{
	int i = m_nPageSize + GetTopIndex() - 1;
	if ( i >= GetCount() )
		i = GetCount() - 1;
	if ( i < 0 )
		i = 0;
	return i;
}

int CDualListBox::GetViewLineLen( int nResPixels ) const
{
	if ( m_hWnd == NULL )
		return 0;
	CRect rect;
	GetClientRect( rect );
	return (rect.Width() - nResPixels) / m_nCharWidth;
}



void CDualListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );

	BOOL bOutside;
	UINT nItem = ItemFromPoint( point, bOutside );
	if ( bOutside )
		nItem = LB_ERR;

	CRect rect;
	GetItemRect(nItem, rect);
	CRect rectLeft = rect;
	rectLeft.left = m_nOffsLeft;
	rectLeft.right = m_nOffsRight;
	CRect rectRight = rect;
	rectRight.left = m_nOffsRight;

	CViewFileSync::Side nOldSide = m_pView->GetSide();
	if ( rectLeft.PtInRect( point ) )
	{
		m_pView->OnListDblClk(nItem, CViewFileSync::left, rectLeft, 
			CPoint(point.x - m_nOffsLeft, point.y - rect.top) );
	}
	else if ( rectRight.PtInRect( point ) )
	{
		m_pView->OnListDblClk(nItem, CViewFileSync::right, rectRight,
			CPoint(point.x - m_nOffsRight, point.y - rect.top) );
	}

//	CListBox::OnLButtonDblClk(nFlags, point);
}
/*
void CDualListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );

	BOOL bOutside;
	UINT nItem = ItemFromPoint( point, bOutside );
	if ( bOutside )
		nItem = LB_ERR;

	CRect rect;
	GetItemRect(nItem, rect);
	CRect rectLeft = rect;
	rectLeft.left = m_nOffsLeft;
	rectLeft.right = m_nOffsRight;
	CRect rectRight = rect;
	rectRight.left = m_nOffsRight;

	CViewFileSync::Side nOldSide = m_pView->GetSide();
	if ( rectLeft.PtInRect( point ) )
	{
		m_pView->OnListRButtonDown(nItem, CViewFileSync::left, rectLeft);
	}
	else if ( rectRight.PtInRect( point ) )
	{
		m_pView->OnListRButtonDown(nItem, CViewFileSync::right, rectRight);
	}

	CListBox::OnRButtonDown(nFlags, point);
}
*/
void CDualListBox::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_pView->OnListVScroll();

//	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);

	int nPosOld = GetScrollPos( SB_VERT );
	switch ( nSBCode )
	{
	case SB_TOP:
		SetTopIndex( 0 );
		break;

	case SB_BOTTOM:
		SetTopIndex( m_nItems );
		break;

	case SB_LINEUP:
		if ( nPosOld > 0 )
		{
			SetTopIndex( nPosOld-1, FALSE );
			ScrollWindow( 0, m_nItemHeight );
			UpdateWindow();
		}
		break;

	case SB_LINEDOWN:
		SetTopIndex( nPosOld+1, FALSE );
		if ( GetScrollPos( SB_VERT ) != nPosOld )
		{
			ScrollWindow( 0, -m_nItemHeight );
			CRect rect;
			GetClientRect( rect );
			rect.bottom -= m_nItemHeight;
			rect.top = rect.bottom - m_nItemHeight;
			InvalidateRect( rect );
			UpdateWindow();
		}
		break;

	case SB_PAGEUP:
		if ( nPosOld > 0 )
		{
			SetTopIndex( nPosOld-m_nPageSize );
		}
		break;

	case SB_PAGEDOWN:
		SetTopIndex( nPosOld+m_nPageSize );
		break;

	case SB_THUMBTRACK:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			if ( GetScrollInfo( SB_VERT, &si, SIF_TRACKPOS ) )
				SetTopIndex( si.nTrackPos );		// nPos;
		}
		break;

	default:
		break;
	}
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDualListBox::AddItems( int nItems )
{
//	for ( int n = 0; n < nItems; ++n )
//	{
//		AddString( NULL );
//	}
	m_nItems += nItems;
	ASSERT( m_nItems >= 0 );
	SetScrollRange( SB_VERT, 0, m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize );
}

// ############### CListBox methods to be simulated

BOOL CDualListBox::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	return CWnd::Create( NULL, _T("DualListBox"), WS_CHILD | WS_VISIBLE | WS_VSCROLL, rect, pParentWnd, nID );
}

int CDualListBox::InitStorage( int nItems, UINT nBytes )
{
	return nItems;	// dummy
}

int CDualListBox::AddString( LPCTSTR lpszItem )
{
	int n = m_nItems++;
	SetScrollRange( SB_VERT, 0, m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize );
	return n;
}

int CDualListBox::InsertString( int index, LPCTSTR lpszItem )
{
	if ( index < 0 )
		return AddString( lpszItem );

	++m_nItems;
	SetScrollRange( SB_VERT, 0, m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize );
	return index;
}

int CDualListBox::DeleteString( UINT nIndex )
{
	if ( (int)nIndex >= m_nItems )
		return LB_ERR;

	--m_nItems;
	SetScrollRange( SB_VERT, 0, m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize );
	return m_nItems;
}

void CDualListBox::ResetContent( )
{
	m_nItems = 0;
	SetScrollRange( SB_VERT, 0, 0 );
	m_nSelFirst = -1;
	m_nSelLast = -1;
	m_nAnchor = -1;
	m_nCaret = -1;
}

int CDualListBox::GetCount( ) const
{
	return m_nItems;
}

int CDualListBox::GetSelCount( ) const
{
	if ( m_nSelFirst < 0 )
		return 0;

	return m_nSelLast - m_nSelFirst + 1;
}

int CDualListBox::SetSel( int nIndex, BOOL bSelect /* = TRUE */ )
{
	if ( nIndex < 0 || nIndex >= m_nItems )
		return LB_ERR;

	if ( bSelect )
	{
		m_nSelFirst = m_nSelLast = nIndex;
	}
	else
	{
		m_nSelFirst = m_nSelLast = -1;
	}
	return 0;
}

int CDualListBox::SetTopIndex( int nIndex, BOOL bInvalidate /* = TRUE */ )
{
	if ( nIndex < 0 )
		nIndex = 0;

	int nMax = m_nItems < m_nPageSize ? 0 : m_nItems - m_nPageSize;
	SetScrollPos( SB_VERT, nIndex < nMax ? nIndex : nMax );
	if ( bInvalidate )
		Invalidate();
	return 0;
}

void CDualListBox::SetAnchorIndex( int nIndex )
{
	m_nAnchor = nIndex;
}

int CDualListBox::SetCaretIndex( int nIndex, BOOL bScroll /* = TRUE */ )
{
	if ( nIndex >= m_nItems )
		nIndex = m_nItems - 1;

	if ( nIndex < 0 )
		return LB_ERR;

	int nTop = GetTopIndex();
	if ( nIndex < nTop )
	{
		SetTopIndex( nIndex );
	}
	else if ( nIndex > nTop + m_nPageSize )
	{
		SetTopIndex( nIndex - m_nPageSize - 1 );
	}
	m_nCaret = nIndex;
	return 0;
}

int CDualListBox::SelItemRange( BOOL bSelect, int nFirstItem, int nLastItem )
{
	if ( nFirstItem < 0 || nFirstItem >= m_nItems || nLastItem < 0 || nLastItem >= m_nItems || nLastItem < nFirstItem )
		return LB_ERR;

	if ( bSelect )
	{
		m_nSelFirst = nFirstItem;
		m_nSelLast = nLastItem;
	}
	else
	{
		m_nSelFirst = m_nSelLast = -1;
	}
	return 0;
}

UINT CDualListBox::ItemFromPoint( CPoint pt, BOOL& bOutside ) const
{
	CRect rect;
	GetClientRect( rect );
	if ( rect.PtInRect( pt ) )
	{
		bOutside = FALSE;
	}
	else
	{
		bOutside = TRUE;
		if ( pt.y < 0 )
			pt.y = 0;
		if ( pt.y > rect.bottom )
			pt.y = rect.bottom-1;
	}
	UINT n = GetTopIndex() + pt.y / m_nItemHeight;
	if ( n >= (UINT)m_nItems )
	{
		bOutside = TRUE;
		n = m_nItems - 1;
	}
	return n;
}

int CDualListBox::GetItemRect( int nIndex, LPRECT lpRect ) const
{
	int nTop = GetTopIndex();
	if ( nIndex < nTop )
		return LB_ERR;

	if ( nIndex - nTop > m_nPageSize )
		return LB_ERR;

	GetClientRect( lpRect );
	lpRect->top += (nIndex - nTop) * m_nItemHeight;
	lpRect->bottom = lpRect->top + m_nItemHeight;
	return 0;
}


void CDualListBox::OnPaint()
{
//	MEASUREITEMSTRUCT mi;
//	mi.CtlType = ODT_LISTBOX;
//	MeasureItem( &mi );

	CRect rectUpd;
	if ( !GetUpdateRect( rectUpd ) )
	{
		GetClientRect( rectUpd );
	}

    CPaintDC dc(this); // device context for painting
	CFont* pOldFont = dc.SelectObject( m_pFont );

	int nTop = GetTopIndex();
	int nMax = nTop + m_nPageSize;
	if ( nMax > m_nItems )
		nMax = m_nItems;
	DRAWITEMSTRUCT di;
	di.CtlType = ODT_LISTBOX;
	di.hDC = dc.m_hDC;
	GetItemRect( nTop, &di.rcItem );
	for ( int i = nTop; i < nMax; ++i )
	{
		if ( di.rcItem.bottom > rectUpd.top && di.rcItem.top < rectUpd.bottom )
		{
			di.itemID = i;
			di.itemAction = 0;	// ODA_FOCUS, ODA_SELECT
			di.itemState = 0;
			if ( i == m_nCaret )
				di.itemState |= ODS_FOCUS;
			if ( m_nSelFirst >= 0 && i >= m_nSelFirst && i <= m_nSelLast )
				di.itemState |= ODS_SELECTED;
			DrawItem( &di );
		}
		di.rcItem.top = di.rcItem.bottom;
		di.rcItem.bottom += m_nItemHeight;
	}
	dc.SelectObject( pOldFont );
}

BOOL CDualListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_pView->OnListVScroll();
	int nPosOld = GetScrollPos( SB_VERT );
	if ( zDelta > 0 ) {
		if ( nPosOld > 0 )
		{
			SetTopIndex( nPosOld-1, FALSE );
			ScrollWindow( 0, m_nItemHeight );
			UpdateWindow();
		}
	}
	else {
		SetTopIndex( nPosOld+1, FALSE );
		if ( GetScrollPos( SB_VERT ) != nPosOld )
		{
			ScrollWindow( 0, -m_nItemHeight );
			CRect rect;
			GetClientRect( rect );
			rect.bottom -= m_nItemHeight;
			rect.top = rect.bottom - m_nItemHeight;
			InvalidateRect( rect );
			UpdateWindow();
		}
	}
	return TRUE;
}
