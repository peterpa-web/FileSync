// ViewList.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "DocManFileSync.h"

#include "ViewList.h"


// CViewList

IMPLEMENT_DYNAMIC(CViewList, CViewFileSync)

CViewList::CViewList()
{
	TRACE0( "CViewList::CViewList()\n" );
}

CViewList::~CViewList()
{
}

BEGIN_MESSAGE_MAP(CViewList, CViewFileSync)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CViewList::DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class for listbox
}

//void CViewList::DrawTreeItem(CDC* pDC, LPNMTVCUSTOMDRAW lplvcd)
//{
//	ASSERT( FALSE );
//	AfxThrowNotSupportedException( );	// should be implemented in derived class for treectrl
//}


// CViewList message handlers

void CViewList::OnInitialUpdate2()
{
	VERIFY( m_buttonsLeft [0].Create(this, ID_FILE_OPEN_LEFT,  IDB_OPEN) );
	VERIFY( m_buttonsRight[0].Create(this, ID_FILE_OPEN_RIGHT, IDB_OPEN) );
	VERIFY( m_buttonsLeft [1].Create(this, ID_FILE_MODELEFT,  0U, IDB_MODE_DIS) );
	VERIFY( m_buttonsRight[1].Create(this, ID_FILE_MODERIGHT, 0U, IDB_MODE_DIS) );
	VERIFY( m_buttonsLeft [2].Create(this, ID_FILE_SAVE_LEFT,  IDB_SAVE, IDB_SAVE_DIS) );
	VERIFY( m_buttonsRight[2].Create(this, ID_FILE_SAVE_RIGHT, IDB_SAVE, IDB_SAVE_DIS) );
//	CRect rectB(0,0,20,20);
//	VERIFY( m_buttonOpenLeft.Create( _T("Open left"), WS_VISIBLE | 
//					BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW, rectB, this, ID_FILE_OPEN_LEFT ) );
//	m_buttonOpenLeft.LoadBitmaps( IDB_OPEN, IDB_OPEN_ACT );
//	VERIFY( m_buttonOpenRight.Create( _T("Open right"), WS_VISIBLE | 
//					BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW, rectB, this, ID_FILE_OPEN_RIGHT ) );
//	m_buttonOpenRight.LoadBitmaps( IDB_OPEN, IDB_OPEN_ACT );

	CRect rect(5,50,200,250);
	m_list.PrepareFont( &m_fontList );
	VERIFY( m_list.Create( WS_VISIBLE | WS_VSCROLL | LBS_DISABLENOSCROLL| LBS_NOTIFY | LBS_NODATA | 
							LBS_EXTENDEDSEL | LBS_OWNERDRAWFIXED, rect, this, IDC_LIST ) );
	m_list.SetView( this );

	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	CDocTemplFileSync *pTemplate = pDM->FindTemplate( m_nTitleID );
	m_comboDirLeft.SetTemplate( pTemplate );
	m_comboDirRight.SetTemplate( pTemplate );
}

void CViewList::MoveClient( LPCRECT lpRect )
{
	m_list.MoveWindow( lpRect );
}

void CViewList::OnListDblClk(UINT nItem, int nSide, const CRect &rect, CPoint point)
{
	// dummy
}

void CViewList::OnListLButtonDown()
{
	SetFocus();
}

void CViewList::OnListVScroll()
{
	// dummy
}

BOOL CViewList::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
    // We don't handle anything but scrolling.
    if ((nFlags & MK_CONTROL) != 0)
        return FALSE;

	// http://code.google.com/p/windjview-subpix/source/browse/trunk/MyScrollView.cpp
	return m_list.OnMouseWheel(nFlags, zDelta, point);

    //return FALSE;
}





