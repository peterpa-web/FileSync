#include "StdAfx.h"
#include "FileSync.h"
#include "DualListBox.h"
#include "DocHex.h"
#include "ColorsDlg.h"
#include "MainFrm.h"

#include "ViewHex.h"

IMPLEMENT_DYNCREATE(CViewHex, CViewList)

CViewHex::CViewHex(void)
{
	m_nTitleID = IDR_VIEWHEX;
	m_nMenueID = IDR_VIEWHEX;
	m_nToolbarID[0] = IDR_VIEWHEX;
	m_nToolbarID[1] = IDR_VIEWHEX;

	m_nWidthLineNo = 50;
	m_nCharOffs = 0;
}

CViewHex::~CViewHex(void)
{
}

BEGIN_MESSAGE_MAP(CViewHex, CViewList)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXT_DIFF, OnUpdateViewNextDiff)
	ON_COMMAND(ID_VIEW_NEXT_DIFF, OnViewNextDiff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREV_DIFF, OnUpdateViewPrevDiff)
	ON_COMMAND(ID_VIEW_PREV_DIFF, OnViewPrevDiff)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(IDC_SEARCH_C, OnSearch)
	ON_COMMAND(ID_SEARCH_B, OnSearchB)
	ON_COMMAND(ID_SEARCH_F, OnSearchF)
	ON_COMMAND(ID_HELP_CONTEXT, &CViewHex::OnHelpContext)
END_MESSAGE_MAP()


void CViewHex::DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	DRAWITEMSTRUCT dis = *lpDrawItemStruct;
	dis.rcItem.right /= 2;
	DrawLBItemSide( pDC, &dis, left );
	dis.rcItem.left = dis.rcItem.right;
	dis.rcItem.right = lpDrawItemStruct->rcItem.right;
	DrawLBItemSide( pDC, &dis, right );
}

void CViewHex::DrawLBItemSide(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct, int nSide)
{
	int nItem = lpDrawItemStruct->itemID;
	const CPos &pos = m_aItemData[nItem].d[nSide];
	BOOL bMark = m_aItemData[nItem].bMark;
	CDocHex *pDoc = GetDoc(nSide);
	CString strLineNo = pDoc->GetLineNo( pos.offs );
	CString strLine = pDoc->GetDisplayLine( pos.offs, pos.len );

	CRect rectNo = lpDrawItemStruct->rcItem;
	CRect rectText = lpDrawItemStruct->rcItem;
	rectNo.right = rectNo.left + m_nWidthLineNo;
	rectText.left = rectNo.right;

	COLORREF crOldTextColor = pDC->GetTextColor();
	COLORREF crOldBkColor = pDC->GetBkColor();

	// If this item is selected, set the background color 
	// and the text color to appropriate values. Also, erase
	// rect by filling it with the background color.
	BOOL bHigh;
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED) &&
		nSide == s_nSide )
	{
		bHigh = TRUE;
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, 
			::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		bHigh = FALSE;
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		if ( bMark )
//			pDC->SetBkColor( m_crMarkDir );
			pDC->FillSolidRect( rectNo, m_crMarkDir );
		else
//			pDC->SetBkColor( m_crBkColor );
			pDC->FillSolidRect( rectNo, m_crBkColor );
		if ( bMark )
			pDC->FillSolidRect(rectText, m_crMark);
		else
			pDC->FillSolidRect(rectText, m_crWndColor);
	}
	// Draw line number.
	if ( pos.len != 0 )
	{
		rectNo.right -= 2;
		pDC->DrawText(
			strLineNo,
			rectNo,
			DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
	}
	// Draw the text.
	if ( !bHigh )
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	int p = 0;
	int nLen = strLine.GetLength();
	if ( nLen >= m_nCharOffs )
	{
		if ( !bHigh && bMark )
		{
			pDC->SetTextColor( m_crTxtChanged );
		}
		LPCTSTR lpszText = (LPCTSTR)strLine;
		pDC->DrawText(
			lpszText + m_nCharOffs,
			nLen - m_nCharOffs,
			rectText,
			DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}

	if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
		(lpDrawItemStruct->itemState & ODS_FOCUS))
    {
        pDC->DrawFocusRect( &(lpDrawItemStruct->rcItem) );
    }
}

void CViewHex::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// TODO: Add your specialized code here and/or call the base class
}

void CViewHex::AddItems( CItemDataArray *paItems )
{
	int nItem = (int)m_aItemData.GetSize();
	for ( int n = 0; n < paItems->GetSize(); ++n )
	{
		m_list.AddString( NULL );
	}
	m_aItemData.InsertAt( nItem, paItems );
}

void CViewHex::InsertItem( int nItem )
{
	ItemData d = { {NULL, NULL}, TRUE };
	InsertItem( nItem, d );
}

void CViewHex::InsertItem( int nItem, const ItemData &d )
{
	m_list.InsertString( nItem, NULL );
	m_aItemData.InsertAt( nItem, d );
}

void CViewHex::InsertItems( int nItem, CItemDataArray *paItems )
{
	for ( int n = 0; n < paItems->GetSize(); ++n )
	{
		m_list.InsertString( nItem, NULL );
	}
	m_aItemData.InsertAt( nItem, paItems );
}

void CViewHex::DeleteItems( int nItem, int nCount /* = 1 */ )
{
	m_list.EnableDraw( FALSE );
	for ( int n = 0; n < nCount; ++n )
	{
		m_list.DeleteString( nItem );
	}
	m_aItemData.RemoveAt( nItem, nCount );
	m_list.EnableDraw( TRUE );
}

void CViewHex::DeleteContents()
{
	CViewFileSync::DeleteContents();
	DeleteContents2();
}

void CViewHex::DeleteContents2()
{
	m_list.ResetContent();
	m_aItemData.RemoveAll();
	SetScrollPos( SB_HORZ, 0, FALSE );
	m_nCharOffs = 0;
	UpdateHScroll();
}

BOOL CViewHex::CompareView()
{
	CWaitCursor waitCursor;
	DeleteContents2();
	m_nItem = -1;
	m_nMark = -1;	// undefined

	TRACE( "Compare start\n" );
	CDocHex *pDocN = GetDoc(0);
	CDocHex *pDocO = GetDoc(1);
	changeNotify[0].MonitorFile( pDocN->GetBasePathName(), pDocN->GetPathName() );
	changeNotify[1].MonitorFile( pDocO->GetBasePathName(), pDocO->GetPathName() );
	int nMax = pDocN->GetDataSize();
	int oMax = pDocO->GetDataSize();
	int ni = 0;
	int oi = 0;
	int n = 0;
	int o = 0;
	while ( n < nMax || o < oMax )
	{
		int en = n;
		int eo = o;
		while ( en < nMax && eo < oMax && pDocN->IsEqual( en, pDocO, eo ) )
		{
			++en;
			++eo;
		}
		TRACE2( "CViewHex::Compare EQ %d %d\n", en-n, eo-o );
		AddItems( n, en, o, eo, FALSE );
		n = en;
		o = eo;
		int mn = n;
		int mo = o;
		pDocN->FindMatchUnique ( mn, ni, pDocO, mo, oi );
		int bn = mn - 1;
		int bo = mo - 1;
		while ( bn >= n && bo >= o && pDocN->IsEqual( bn, pDocO, bo ) )
		{
			--bn;
			--bo;
		}
		++bn;
		++bo;
		TRACE2( "CViewHex::Compare MA %d %d\n", bn-n, bo-o );
		AddItems( n, bn, o, bo, TRUE );
//		TRACE2( "CViewHex::Compare BE %d %d\n", mn-bn, mo-bo );
//		AddItems( bn, mn, bo, mo, FALSE );
		n = bn;
		o = bo;
	}

	CItemDataArray aItems;
	AddItems( &aItems );
	m_pProgressMan->ResetUserAbortFlag();
	m_pProgressMan->SetVisible( FALSE );
	return TRUE;
}

void CViewHex::AddItems( int n, int nMax, int o, int oMax, BOOL bMark )
{
	int cn = nMax - n;
	int co = oMax - o;
	TRACE3( "CViewHex::AddItems %X %X %d\n", n, o, bMark );
	while ( cn != 0 || co != 0 )
	{
		if ( bMark != m_nMark )
		{
			ItemData d = { { { n, 0 }, { o, 0 } }, bMark };
			m_nItem = m_list.AddString( NULL );
			m_aItemData.InsertAt( m_nItem, d );
			m_nMark = bMark;
		}
		ItemData &d = m_aItemData[ m_nItem ];
		ASSERT( (d.d[0].offs+d.d[0].len) == n );
		ASSERT( (d.d[1].offs+d.d[1].len) == o );
		int ln = n & 0xf;
		int lo = o & 0xf;

		int cn2 = cn;
		if ( (cn2 + ln) > 0x10 )
			cn2 = 0x10 - ln;
		d.d[0].len += cn2;
		cn -= cn2;
		n += cn2;

		int co2 = co;
		if ( (co2 + lo) > 0x10 )
			co2 = 0x10 - lo;
		d.d[1].len += co2;
		co -= co2;
		o += co2;

		if ( cn != 0 || co != 0 )
			m_nMark = -1;	// force insert
	}
}

void CViewHex::OnUpdateViewNextDiff(CCmdUI *pCmdUI)
{
	if ( m_list.m_hWnd == NULL )
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	int nTop = m_list.GetBottomIndex();
	int nTopOld = nTop;
	// find next non marked line
	while ( nTop < m_list.GetCount() && m_aItemData[nTop].bMark )
		++nTop;
	// find first marked line
	while ( nTop < m_list.GetCount() && !m_aItemData[nTop].bMark )
		++nTop;
	if ( nTop > nTopOld && nTop < m_list.GetCount() &&  m_aItemData[nTop].bMark )
		pCmdUI->Enable();
	else
		pCmdUI->Enable(FALSE);
}

void CViewHex::OnViewNextDiff()
{
	int nTop = m_list.GetBottomIndex();
	int nTopOld = nTop;
	// find next non marked line
	while ( nTop < m_list.GetCount() && m_aItemData[nTop].bMark )
		++nTop;
	// find first marked line
	while ( nTop < m_list.GetCount() && !m_aItemData[nTop].bMark )
		++nTop;
	if ( nTop > nTopOld && nTop < m_list.GetCount() && m_aItemData[nTop].bMark )
	{
		m_list.SetTopIndex( nTop );
	}
}

void CViewHex::OnUpdateViewPrevDiff(CCmdUI *pCmdUI)
{
	int nTop = 0;
	if ( m_list.m_hWnd != NULL )
		nTop = m_list.GetTopIndex();

	for ( --nTop; nTop >= 0; --nTop )
	{
		if ( m_aItemData[nTop].bMark )
		{
			pCmdUI->Enable(TRUE);
			return;
		}
	}
	pCmdUI->Enable(FALSE);
}

void CViewHex::OnViewPrevDiff()
{
	int nTop = m_list.GetTopIndex();
	int nTopOld = nTop;
	// find prev non marked line
	while ( nTop >= 0 && m_aItemData[nTop].bMark  )
		--nTop;
	// find prev last marked line
	while ( nTop >= 0 && !m_aItemData[nTop].bMark )
		--nTop;
	if ( nTop < 0 )
	{
		++nTop;
		// find first marked line
		while ( nTop < nTopOld && !m_aItemData[nTop].bMark )
			++nTop;
	}
	else
	{
		// find prev non marked line
		while ( nTop >= 0 && m_aItemData[nTop].bMark )
			--nTop;
		++nTop;		// possible first marked line
	}
	if ( nTop < nTopOld && m_aItemData[nTop].bMark )
	{
		m_list.SetTopIndex( nTop );
	}
}

void CViewHex::UpdateHScroll()
{
	SCROLLINFO si;
	if ( !GetScrollInfo( SB_HORZ, &si ) )
	{
		si.cbSize = sizeof(si);
		si.nPos = 0;
		si.nTrackPos = 0;
	}
	si.nMin = 0;
	si.nMax = 0;
	if ( m_pDoc[0] != NULL )
		si.nMax = m_pDoc[0]->GetMaxLineLen();
	if ( m_pDoc[1] != NULL && m_pDoc[1]->GetMaxLineLen() > si.nMax )
		si.nMax = m_pDoc[1]->GetMaxLineLen();
	if ( si.nPos > si.nMax )
		si.nPos = si.nMax;
	si.nPage = m_list.GetViewLineLen( m_nWidthLineNo * 2 ) / 2;
	if ( si.nMax > (int)si.nPage )
	{
		if ( si.nPos > si.nMax )
			si.nPos = si.nMax;
	}
	else
	{
		si.nMax = 0;
		si.nPage = 0;
		si.nPos = 0;
	}
//		si.fMask = SIF_DISABLENOSCROLL;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	TRACE3( "CViewHex::UpdateHScroll nMax=%d nPage=%d nPos=%d\n", si.nMax, si.nPage, si.nPos );
	SetScrollInfo( SB_HORZ, &si );
}


void CViewHex::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if ( pScrollBar != NULL )
	{
		CViewFileSync::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

//	if ( pScrollBar->GetDlgCtrlID() != SB_HORZ )
//		return;

	SCROLLINFO si;
	VERIFY( GetScrollInfo( SB_HORZ, &si ) );
    int nPosNew = si.nPos;
	TRACE1( "OnHScroll code=%d\n", nSBCode );

	switch( nSBCode )
	{
	case SB_RIGHT:	    // 7 VK_END
		nPosNew = si.nMax;
		break;
//	case SB_ENDSCROLL:	// 8 WM_KEYUP (the user released a key that sent a relevant virtual-key code)
	case SB_LINERIGHT:	// 1 VK_RIGHT or VK_DOWN
		if ( nPosNew < si.nMax )
			++nPosNew;
		break;
	case SB_LINELEFT:	    // 0 VK_LEFT or VK_UP
		if ( nPosNew > 0 )
			--nPosNew;
		break;
	case SB_PAGERIGHT:	// 3 VK_NEXT (the user clicked the channel below or to the right of the slider)
		if ( nPosNew < (si.nMax-(int)si.nPage) )
			nPosNew += si.nPage;
		else
			nPosNew = si.nMax;
		break;
	case SB_PAGELEFT:	    // 2 VK_PRIOR (the user clicked the channel above or to the left of the slider)
		if ( nPosNew > (int)si.nPage )
			nPosNew -= si.nPage;
		else
			nPosNew = 0;
		break;
//	case SB_THUMBPOSITION:	// 4 WM_LBUTTONUP following a TB_THUMBTRACK notification message
	case SB_THUMBTRACK:	// 5 Slider movement (the user dragged the slider)
		nPosNew = nPos;
		break;
	case SB_LEFT:	    // 6 VK_HOME
		nPosNew = 0;
		break;
	}
	SetScrollPos( SB_HORZ, nPosNew );
	m_nCharOffs = nPosNew;
	m_list.Invalidate();
	TRACE1( "OnHScroll %d\n", nPosNew );

//	CViewFileSync::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CViewHex::OnSize(UINT nType, int cx, int cy)
{
	CViewFileSync::OnSize(nType, cx, cy);

	UpdateHScroll();
}

int CViewHex::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_strColorsSection = "Hex";
	CColorsDlg dlg;
	dlg.GetProfile( m_strColorsSection );
	m_crMarkDir = dlg.m_acr[0];
	m_crMarkLite = dlg.m_acr[1];
	m_crMark = dlg.m_acr[2];
	m_crTxtChanged = dlg.m_acr[3];
	m_crMarkSingle = dlg.m_acr[4];

	if (CViewFileSync::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CViewHex::OnSearch()
{
	OnSearchF();
}

void CViewHex::OnSearchB()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CString strSearch = pFrame->GetSearchText().MakeLower();
	if ( strSearch.IsEmpty() )
		return;

	int nStart = 0;
	int nEnd = m_list.GetCount()-1;
	int nCount = m_list.GetSelCount();
	if ( nCount >= 1 ) {
		int nSelAnchor = m_list.GetAnchorIndex();
		int nSelCaret = m_list.GetCaretIndex();
		if ( nSelAnchor == LB_ERR )
			nSelAnchor = nSelCaret;
		ASSERT( nSelCaret != LB_ERR );

		if ( nSelAnchor < nSelCaret )
		{
			nStart = nSelAnchor;
		} else {
			nStart = nSelCaret;
		}
		--nStart;
	}

	CDocHex *pDoc = GetDoc(s_nSide);
	for ( int nItem = nStart; nItem >= 0; --nItem )
	{
		CString str;
		const CPos &pos = m_aItemData[nItem].d[s_nSide];
		if ( pDoc->SearchLine( strSearch, pos.offs, pos.len ) )
		{
			if ( nItem < m_list.GetTopIndex() )
				m_list.SetTopIndex( nItem );
			m_list.SelItemRange( FALSE, 0, nEnd );
			m_list.SetAnchorIndex( nItem );
			m_list.SetCaretIndex( nItem );
			m_list.SetSel( nItem );
			m_list.Invalidate();
			return;
		}
	}
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

void CViewHex::OnSearchF()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CString strSearch = pFrame->GetSearchText().MakeLower();
	if ( strSearch.IsEmpty() )
		return;

	int nStart = 0;
	int nEnd = m_list.GetCount()-1;
	int nCount = m_list.GetSelCount();
	if ( nCount >= 1 ) {
		int nSelAnchor = m_list.GetAnchorIndex();
		int nSelCaret = m_list.GetCaretIndex();
		if ( nSelAnchor == LB_ERR )
			nSelAnchor = nSelCaret;
		ASSERT( nSelCaret != LB_ERR );

		if ( nSelAnchor < nSelCaret )
		{
			nStart = nSelAnchor;
		} else {
			nStart = nSelCaret;
		}
		++nStart;
		ASSERT( nStart != LB_ERR );
	}

	CDocHex *pDoc = GetDoc(s_nSide);
	for ( int nItem = nStart; nItem < nEnd; ++nItem )
	{
		CString str;
		const CPos &pos = m_aItemData[nItem].d[s_nSide];
		if ( pDoc->SearchLine( strSearch, pos.offs, pos.len ) )
		{
			if ( nItem > m_list.GetBottomIndex() )
				m_list.SetTopIndex( nItem );
			m_list.SelItemRange( FALSE, 0, nEnd );
			m_list.SetAnchorIndex( nItem );
			m_list.SetCaretIndex( nItem );
			m_list.SetSel( nItem );
			m_list.Invalidate();
			return;
		}
	}
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

void CViewHex::OnHelpContext()
{
	CFileSyncApp* pApp = (CFileSyncApp*)AfxGetApp();
	ShellExecute( GetParentFrame()->m_hWnd, _T("open"), _T("viewhex.html"), NULL, pApp->m_strHelpPath, SW_SHOW );
}
