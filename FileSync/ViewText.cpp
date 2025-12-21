#include "StdAfx.h"
#include "FileSync.h"
#include "DualListBox.h"
#include "DocText.h"
#include "PrefDlg.h"
#include "UndoInsertText.h"
#include "UndoDelete.h"
#include "UndoInsertAfter.h"
#include "UndoInsertBefore.h"
#include "UndoReplace.h"
#include "UndoReplaceText.h"
#include "ColorsDlg.h"
#include "EditLine.h"
#include "MainFrm.h"

#include "ViewText.h"

IMPLEMENT_DYNCREATE(CViewText, CViewList)

CViewText::CViewText(void)
{
	m_nTitleID = IDR_VIEWTEXT;
	m_nMenueID = IDR_VIEWTEXT;
	m_nToolbarID[0] = IDR_VIEWTEXT;
	m_nToolbarID[1] = IDR_VIEWTEXT_R;

//	m_nWidthLineNo = 42;
	m_nCharOffs = 0;
	m_pEdit = NULL;

	m_undoBuffer.SetMaxSize( 10000000 );
}

CViewText::~CViewText(void)
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
	}
}

BEGIN_MESSAGE_MAP(CViewText, CViewFileSync)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXT_DIFF, OnUpdateViewNextDiff)
	ON_COMMAND(ID_VIEW_NEXT_DIFF, OnViewNextDiff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREV_DIFF, OnUpdateViewPrevDiff)
	ON_COMMAND(ID_VIEW_PREV_DIFF, OnViewPrevDiff)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERTAFTER, OnUpdateEditInsertafter)
	ON_COMMAND(ID_EDIT_INSERTAFTER, OnEditInsertafter)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERTBEFORE, OnUpdateEditInsertbefore)
	ON_COMMAND(ID_EDIT_INSERTBEFORE, OnEditInsertbefore)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACESEL, OnUpdateEditReplacesel)
	ON_COMMAND(ID_EDIT_REPLACESEL, OnEditReplacesel)
	ON_COMMAND(ID_EDIT_PREF, OnEditPref)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEL_LINES, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_DEL_LINES, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDel)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDel)
	ON_WM_CREATE()
	ON_COMMAND(IDC_EDITLINE, OnEditLineEnter)
	ON_COMMAND(IDC_SEARCH_C, OnSearch)
	ON_COMMAND(ID_SEARCH_B, OnSearchB)
	ON_COMMAND(ID_SEARCH_F, OnSearchF)
	ON_COMMAND(ID_HELP_CONTEXT, &CViewText::OnHelpContext)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXT, &CViewText::OnUpdateViewText)
END_MESSAGE_MAP()


void CViewText::DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	DRAWITEMSTRUCT dis = *lpDrawItemStruct;
	dis.rcItem.right /= 2;
	DrawLBItemSide( pDC, &dis, left );
	dis.rcItem.left = dis.rcItem.right;
	dis.rcItem.right = lpDrawItemStruct->rcItem.right;
	DrawLBItemSide( pDC, &dis, right );
}

void CViewText::DrawLBItemSide(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct, int nSide)
{
	int nItem = lpDrawItemStruct->itemID;
	POSLINE pos = m_aItemData[nItem].pos[nSide];
	BOOL bMark = m_aItemData[nItem].bMark;
	CDocText *pDoc = GetDoc(nSide);
	int nLineNo = pDoc->GetLineNo( pos );
	LPCTSTR lpszText = pDoc->GetDisplayLine( pos );
	int nLen = pDoc->GetDisplayLineLen( pos );
//	BOOL bUnique = pDoc->IsUnique( pos );

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
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
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
		if ( nLineNo == 0 )
			pDC->FillSolidRect( rectNo, m_crMarkSingle );
		else if ( bMark )
			pDC->FillSolidRect( rectNo, m_crMarkDir );
		else {
//			if ( bUnique )
//				pDC->FillSolidRect( rectNo, m_crMarkLite );
//			else
				pDC->FillSolidRect( rectNo, m_crBkColor );
		}
		if ( bMark )
			pDC->FillSolidRect(rectText, m_crMarkLite);
		else
			pDC->FillSolidRect(rectText, m_crWndColor);
	}
	// Draw line number.
	if ( nLineNo > 0 )
	{
		rectNo.right -= 2;
		CString strLineNo;
		strLineNo.Format( _T("%d"), nLineNo );
		pDC->DrawText(
			strLineNo,
			rectNo,
			DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
	}
	// Draw the text.
	if ( !bHigh )
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	int p = 0;
	if ( nLen >= m_nCharOffs )
	{
		if ( !bMark )
		{
			pDC->DrawText(
				lpszText + m_nCharOffs,
				nLen - m_nCharOffs,
				rectText,
				DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		}
		else
		{
			CRect rectChar = rectText;
			int nCharWidth = m_list.GetCharWidth();
			rectChar.right = rectChar.left + nCharWidth;
			LPCTSTR pszChar = lpszText + m_nCharOffs;
			CStringDiff &sd = pDoc->GetStringDiff( pos );
			nLen -= m_nCharOffs;
			int p = m_nCharOffs;
			while ( nLen > 0 && rectChar.left < rectText.right )
			{
				if ( sd[p] )
					pDC->FillSolidRect( rectChar, m_crMark );
				if ( bHigh ) {
					if ( sd[p] )
						pDC->SetTextColor( m_crTxtChanged );
					else
						pDC->SetTextColor( ::GetSysColor(COLOR_HIGHLIGHTTEXT) );
				} else {
					if ( sd[p] )
						pDC->SetTextColor( m_crTxtChanged );
					else
						pDC->SetTextColor( ::GetSysColor(COLOR_WINDOWTEXT) );
				}
				pDC->DrawText(
					pszChar,
					1,
					rectChar,
					DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
				++pszChar;
				++p;
				--nLen;
				rectChar.left += nCharWidth;
				rectChar.right += nCharWidth;
			}
		}
	}

	if ( lpDrawItemStruct->itemState & ODS_FOCUS )
    {
        pDC->DrawFocusRect( &(lpDrawItemStruct->rcItem) );
    }
}

void CViewText::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// TODO: Add your specialized code here and/or call the base class
}

void CViewText::AddItems( CItemDataArray *paItems )
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	int nItem = (int)m_aItemData.GetSize();
//	m_list.InitStorage( (int)paItems->GetSize(), 8 );
//	for ( int n = 0; n < paItems->GetSize(); ++n )
//	{
//		m_list.AddString( NULL );
//	}
	m_list.AddItems( paItems->GetSize() );
	TRACE1( "CViewText::AddItems() 1 %dms\n", GetTickCount()-dwTicksStart );
	m_aItemData.InsertAt( nItem, paItems );
	TRACE1( "CViewText::AddItems() e %dms\n", GetTickCount()-dwTicksStart );
}

void CViewText::InsertItem( int nItem )
{
	ItemData d = { {NULL, NULL}, TRUE };
	InsertItem( nItem, d );
}

void CViewText::InsertItem( int nItem, const ItemData &d )
{
	ASSERT(POSLINE_ISVALID(d.pos[0]) || d.pos[0]==NULL);
	ASSERT(POSLINE_ISVALID(d.pos[1]) || d.pos[1]==NULL);
	m_list.InsertString( nItem, NULL );
	m_aItemData.InsertAt( nItem, d );
}

void CViewText::InsertItems( int nItem, CItemDataArray *paItems )
{
	m_list.AddItems( paItems->GetSize() );
	m_aItemData.InsertAt( nItem, paItems );
}

void CViewText::DeleteItems( int nItem, int nCount /* = 1 */ )
{
//	m_list.EnableDraw( FALSE );
	m_list.AddItems( -nCount );
	m_aItemData.RemoveAt( nItem, nCount );
//	m_list.EnableDraw( TRUE );
}

void CViewText::DeleteContents()
{
	CViewFileSync::DeleteContents();
	DeleteContents2();
}

void CViewText::DeleteContents2()
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	if ( m_pEdit != NULL )
	{
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
	}
	m_list.EnableDraw( FALSE );
	m_list.ResetContent();
	m_aItemData.RemoveAll();
	m_undoBuffer.RemoveAll();
	SetScrollPos( SB_HORZ, 0, FALSE );
	m_nCharOffs = 0;
	UpdateHScroll();
	m_list.EnableDraw( TRUE );
	TRACE1( "CViewText::DeleteContents2() %dms\n", GetTickCount()-dwTicksStart );
}

BOOL CViewText::CompareView()
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	CWaitCursor waitCursor;
	DeleteContents2();

	CPosArray aposUniL;
	CPosArray aposUniR;
	CDocText *pDocL = GetDoc(0);
	CDocText *pDocR = GetDoc(1);
	ASSERT( pDocL != NULL );
	ASSERT( pDocR != NULL );
	changeNotify[0].MonitorFile( pDocL->GetBasePathName(), pDocL->GetPathName() );
	changeNotify[1].MonitorFile( pDocR->GetBasePathName(), pDocR->GetPathName() );
	m_buttonsLeft[1].SetDlgCtrlID(ID_FILE_MODELEFT);
	m_buttonsLeft[1].LoadBitmapsEx(GetModeBitmap(left, pDocL->GetPathName().IsEmpty()));
	m_buttonsRight[1].SetDlgCtrlID(ID_FILE_MODERIGHT);
	m_buttonsRight[1].LoadBitmapsEx(GetModeBitmap(right, pDocR->GetPathName().IsEmpty()));
	m_pProgressMan->SetCaption( _T("FileSync Progress") );
//	m_pProgressMan->ResetUserAbortFlag();
//	m_pProgressMan->EnableCancel( TRUE );
//	m_pProgressMan->SetAbortText( _T("Canceled ...") );
	m_pProgressMan->SetStaticText( 0, _T("Comparing ...") );
	m_pProgressMan->SetStaticText( 1, _T("") );
	m_pProgressMan->EnableProgress( TRUE );
	m_pProgressMan->SetProgress( 0 );
	__int64 nRange = 11 * max( pDocL->GetLineCount(), pDocR->GetLineCount() ) / 10;
	m_pProgressMan->SetRange( nRange );
	m_pProgressMan->SetVisible( TRUE );

	POSLINE posL = pDocL->GetFirstLine();
	POSLINE posR = pDocR->GetFirstLine();
//	TRACE( "Compare start\n" );
	TRACE1( "CViewText::CompareView() s %dms\n", GetTickCount()-dwTicksStart );

	CItemDataArray aItems;
	if ( !CompareUniqueBlock1( aItems, pDocL, posL,  NULL, pDocR, posR, NULL ) )
		return FALSE;
	AddItems( &aItems );

	// add empty line
	ItemData d = { {NULL, NULL}, FALSE };
	InsertItem( (int)m_aItemData.GetSize(), d );
	m_pProgressMan->ResetUserAbortFlag();
	m_pProgressMan->SetVisible( FALSE );
	Invalidate();
	TRACE1( "CViewText::CompareView() e %dms\n", GetTickCount()-dwTicksStart );
	return TRUE;
}

BOOL CViewText::CompareUniqueBlock1( CItemDataArray &aItems,
		CDocText *pDocL, POSLINE posL, const POSLINE posLM, 
		CDocText *pDocR, POSLINE posR, const POSLINE posRM )
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	TRACE2( "CViewText::CompareUniqueBlock1 %d %d\n", pDocL->GetLineNo( posL ), pDocR->GetLineNo( posR ) );
	while ( POSLINE_ISVALID(posL) && posL != posLM && POSLINE_ISVALID(posR) && posR != posRM )
	{
		if ( pDocL->CompareLine( posL, pDocR, posR ) )
		{
			ItemData d = { {posL, posR}, FALSE };
			aItems.Add( d );
			pDocL->GetNextLine( posL );
			pDocR->GetNextLine( posR );
			continue;
		}
		{
			LARGE_INTEGER nNull;
			nNull.QuadPart = 0;
			LARGE_INTEGER nTransf;
			nTransf.QuadPart = aItems.GetSize();
			DWORD rc = CopyProgressRoutine( nNull, nTransf, nNull, nNull, 0, CALLBACK_STREAM_SWITCH, NULL, NULL, m_pProgressMan );
			if ( rc == PROGRESS_CANCEL )
				return FALSE;
		}
		// find matching unique lines
		POSLINE posLU = posL;
		POSLINE posRU = posR;
		TRACE2( "  FindMatchUnique1 from %d %d\n", pDocL->GetLineNo( posL ), pDocR->GetLineNo( posR ) );
		if ( !pDocL->FindMatchUnique1( posLU, posLM, pDocR, posRU, posRM ) )
		{
			posLU = posLM;
			posRU = posRM;
		}
		TRACE2( "           Unique1 until %d %d\n", pDocL->GetLineNo( posLU ), pDocR->GetLineNo( posRU ) );
		ASSERT( (DWORD)posLU < 2 || pDocL->GetLineNo( posL ) <= pDocL->GetLineNo( posLU ) );
		ASSERT( (DWORD)posRU < 2 || pDocR->GetLineNo( posR ) <= pDocR->GetLineNo( posRU ) );

		CItemDataArray aSubItems;
		CMapStringToPtr mapL;		// MatchWords
		CMapStringToPtr mapR;
		pDocL->CombineWordsForLines( mapL, posL, posLU );
		pDocR->CombineWordsForLines( mapR, posR, posRU );
		pDocL->MarkLinesFromWordMaps( mapL, pDocR, mapR );
		CPosArray aposL2;
		CPosArray aposR2;
		pDocL->GetUniqueWordsLines( posL, posLU, aposL2 );
		pDocR->GetUniqueWordsLines( posR, posRU, aposR2 );
		int nL = 0;
		int nR = 0;
		CompareUniqueBlock2( aSubItems, pDocL, posL, aposL2, nL, posLU, pDocR, posR, aposR2, nR, posRU );
		aItems.Append( aSubItems );
		TRACE1( " aItems = %d\n", aItems.GetCount() );
		posL = posLU;
		posR = posRU;
	}
	// insert rest
	CItemDataArray aSubItems;
	CompareBlock( aSubItems, pDocL, posL, posLM, pDocR, posR, posRM );
	aItems.Append( aSubItems );

	TRACE0( " Compare update string diffs\n" );
	for ( int n = 0; n < aItems.GetSize(); ++n )
	{
		ItemData &id = aItems[n];
		if ( id.bMark )
		{
			if ( id.pos[0] != NULL )
				pDocL->UpdateStringDiff( id.pos[0], pDocR, id.pos[1] );
			else
				pDocR->UpdateStringDiff( id.pos[1], pDocL, id.pos[0] );
		}
	}
	TRACE1( "CViewText::CompareUniqueBlock1() %dms\n", GetTickCount()-dwTicksStart );
	return TRUE;
}

void CViewText::CompareUniqueBlock2( CItemDataArray &aItems,
							 const CDocText *pDocL, POSLINE posL, CPosArray &aposL, int &nL, const POSLINE posLM, 
							 const CDocText *pDocR, POSLINE posR, CPosArray &aposR, int &nR, const POSLINE posRM )
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	TRACE2( "CViewText::CompareUniqueBlock2 %d %d\n", pDocL->GetLineNo( posL ), pDocR->GetLineNo( posR ) );
	TRACE2( "                            to %d %d\n", pDocL->GetLineNo( posLM ), pDocR->GetLineNo( posRM ) );
	while ( POSLINE_ISVALID(posL) && posL != posLM && POSLINE_ISVALID(posR) && posR != posRM )
	{
		if ( pDocL->CompareWordsLine( posL, posR ) )		// MatchWords
		{
			ItemData d = { {posL, posR}, !pDocL->CompareLine( posL, pDocR, posR ) };
			aItems.Add( d );
			pDocL->GetNextLine( posL );
			pDocR->GetNextLine( posR );
			continue;
		}
		// find matching unique lines
		POSLINE posLU = posL;
		POSLINE posRU = posR;
		if ( !pDocL->FindMatchUniqueWords( posLU, aposL, nL, pDocR, posRU, aposR, nR ) )
			break;

		TRACE2( "Compare found unique2 %d %d\n", pDocL->GetLineNo( posLU ), pDocR->GetLineNo( posRU ) );
		CItemDataArray aSubItems;
		int nMatch = CompareBlock( aSubItems, pDocL, posL, posLU, pDocR, posR, posRU );
		aItems.Append( aSubItems );
		posL = posLU;
		posR = posRU;
	}
	// insert rest
	CItemDataArray aSubItems;
	CompareBlock( aSubItems, pDocL, posL, posLM, pDocR, posR, posRM );
	aItems.Append( aSubItems );
	TRACE1( "CViewText::CompareUniqueBlock1() %dms\n", GetTickCount()-dwTicksStart );
}

int CViewText::CompareBlock( CItemDataArray &aItems,
							 const CDocText *pDocL, POSLINE posL, POSLINE posLU, 
							 const CDocText *pDocR, POSLINE posR, POSLINE posRU )
{
#ifdef _DEBUG
//	DWORD dwTicksStart = GetTickCount();
#endif
	int nMatch = 0;		// return value
	TRACE2( "CViewText::CompareBlock %d %d", pDocL->GetLineNo( posL ), pDocR->GetLineNo( posR ) );
	ASSERT( pDocL != NULL );
	ASSERT( pDocR != NULL );
	int nLinesL = pDocL->GetLineCount( posL, posLU );
	int nLinesR = pDocR->GetLineCount( posR, posRU );
	TRACE2( " lines %d %d \n", nLinesL, nLinesR );
	ASSERT( nLinesL >= 0 );
	ASSERT( nLinesR >= 0 );
	if ( posLU == POSLINE_END )
		posLU = NULL;
	if ( posRU == POSLINE_END )
		posRU = NULL;

	BOOL bComp = FALSE;
	// match upper lines ...
	CItemDataArray aItemsU;
	POSLINE posLB = posLU;
	POSLINE posRB = posRU;
	if ( posL != posLU && posR != posRU )
	{
		if ( !POSLINE_ISVALID(posLB) )
			posLB = pDocL->GetLastLine();
		else
			pDocL->GetPrevLine( posLB, 1 );
		if ( !POSLINE_ISVALID(posRB) )
			posRB = pDocR->GetLastLine();
		else
			pDocR->GetPrevLine( posRB, 1 );

		while ( nLinesL != 0 && nLinesR != 0 )
		{
			ASSERT( nLinesL >= 0 );
			ASSERT( nLinesR >= 0 );
			bComp = pDocL->CompareLine( posLB, pDocR, posRB );
			if ( bComp )
			{
				++nMatch;
				ASSERT(posLB != NULL && posRB != NULL);
				ItemData d = { {posLB, posRB}, FALSE };
				aItemsU.InsertAt( 0, d );
				pDocL->GetPrevLine( posLB, 1 );
				pDocR->GetPrevLine( posRB, 1 );
				--nLinesL;
				--nLinesR;
			}
			else
				break;
		}
		if ( POSLINE_ISVALID(posLB) )
			pDocL->GetNextLine( posLB );
		if ( POSLINE_ISVALID(posRB) )
			pDocR->GetNextLine( posRB );
	}

	// match lower lines ...
	while ( POSLINE_ISVALID(posL) && POSLINE_ISVALID(posR) && (nLinesL > 0 || nLinesR > 0) )
	{
		bComp = FALSE;
		if ( nLinesL > 0 && nLinesR > 0 )
			bComp = pDocL->CompareLine( posL, pDocR, posR );
		if ( bComp )
		{
			++nMatch;
			ItemData d = { {posL, posR}, FALSE };
			aItems.Add( d );
			pDocL->GetNextLine( posL );
			pDocR->GetNextLine( posR );
			--nLinesL;
			--nLinesR;
			continue;
		}

		if ( nLinesL == 0 )
		{
			ItemData d = { {NULL, posR}, TRUE };
			aItems.Add( d );
			pDocR->GetNextLine( posR );
			--nLinesR;
			continue;
		}
		
		if ( nLinesR == 0 )
		{
			ItemData d = { {posL, NULL}, TRUE };
			aItems.Add( d );
			pDocL->GetNextLine( posL );
			--nLinesL;
			continue;
		}
		
		if ( nLinesL == nLinesR )
		{
			ItemData d = { {posL, posR}, TRUE };
			aItems.Add( d );
			pDocL->GetNextLine( posL );
			pDocR->GetNextLine( posR );
			--nLinesL;
			--nLinesR;
			continue;
		}

		int nMatchB;
		if ( nLinesL < nLinesR )
		{
			nMatchB = MatchBlock( aItems, FALSE, pDocL, posL, posLB, nLinesL,
												 pDocR, posR, posRB, nLinesR );
		}
		else
		{
			nMatchB = MatchBlock( aItems, TRUE, pDocR, posR, posRB, nLinesR,
												pDocL, posL, posLB, nLinesL );
		}
		if ( nMatchB >= 0 )
		{
			aItems.Append( aItemsU );
//			TRACE1( "CViewText::CompareBlock() %dms\n", GetTickCount()-dwTicksStart );
			return nMatch + nMatchB;
		}
		InsertBlock( aItems, pDocL, posL, posLB, nLinesL, pDocR, posR, posRB, nLinesR );
	}

	InsertBlock( aItems, pDocL, posL, posLB, nLinesL, pDocR, posR, posRB, nLinesR );
	aItems.Append( aItemsU );
//	TRACE1( "CViewText::CompareBlock() %dms\n", GetTickCount()-dwTicksStart );
	return nMatch;
}

void CViewText::InsertBlock( CItemDataArray &aItems, 
							 const CDocText *pDocL, POSLINE &posL, const POSLINE posLU, int &dL, 
							 const CDocText *pDocR, POSLINE &posR, const POSLINE posRU, int &dR )
{
	TRACE2( "InsertBlock %d %d\n", dL, dR );
	while ( POSLINE_ISVALID(posL) && posL != posLU && POSLINE_ISVALID(posR) && posR != posRU && dL > 0 && dR > 0)
	{
		BOOL bComp = pDocL->CompareLine( posL, pDocR, posR );
		if ( bComp ) {
			ItemData d = { {posL, posR}, FALSE };
			aItems.Add( d );
		}
		else {
			ItemData d = { {posL, posR}, TRUE };
			aItems.Add( d );
		}
		pDocL->GetNextLine( posL );
		pDocR->GetNextLine( posR );
		--dL;
		--dR;
	}
	while ( (!POSLINE_ISVALID(posL) || posL == posLU) && POSLINE_ISVALID(posR) && posR != posRU && dR > 0)
	{
		ItemData d = { {NULL, posR}, TRUE };
		aItems.Add( d );
		pDocR->GetNextLine( posR );
		--dR;
	}
	while ( POSLINE_ISVALID(posL) && posL != posLU && (!POSLINE_ISVALID(posR) || posR == posRU) && dL > 0)
	{
		ItemData d = { {posL, NULL}, TRUE };
		aItems.Add( d );
		pDocL->GetNextLine( posL );
		--dL;
	}
}

int CViewText::MatchBlock( CItemDataArray &aItems, BOOL bRight,
					  const CDocText *pDoc1, POSLINE &pos1, const POSLINE pos1U, int &nLines1, 
					  const CDocText *pDoc2, POSLINE &pos2, const POSLINE pos2U, int &nLines2 )
{
	TRACE2( "MatchBlock %d %d ", pDoc1->GetLineNo( pos1 ), pDoc2->GetLineNo( pos2 ) );
	ASSERT( nLines1 <= nLines2 );
	ASSERT( nLines1 >= 0 );

	POSLINE pos2M = pos2;
	POSLINE pos2V = pos2U;
	CDocText::LineData ld1 = pDoc1->GetLineDataAt( pos1 );
	if ( pos2V == NULL || pos2V == POSLINE_END )
	{
		if ( nLines1 > 1 )
			pos2V = pDoc2->GetLastLine();
		pDoc2->GetPrevLine( pos2V, nLines1 - 2 );
	}
	else
		pDoc2->GetPrevLine( pos2V, nLines1 - 1 );
	if ( !pDoc2->FindMatch( pos2M, pos2V, ld1 ) )
	{
		TRACE1( "no match ld1.nLine=%d\n", ld1.nLine );
		return -1;
	}

	TRACE1( "at %d\n", pDoc2->GetLineNo( pos2M ) );
	struct _m {
		CItemDataArray aItems;
		int nLines1;
		int nLines2;
		POSLINE pos1;
		POSLINE pos2;
		int nMatch;
	} m[2], *pm;
	// 0: ignore match
	pm = m;
	pm->nLines1 = nLines1;
	pm->nLines2 = nLines2;
	pm->pos1 = pos1;
	pm->pos2 = pos2;
	if ( bRight )
	{
		ItemData d = { {pos2, pos1}, TRUE };
		pm->aItems.Add( d );
	}
	else
	{
		ItemData d = { {pos1, pos2}, TRUE };
		pm->aItems.Add( d );
	}
	pDoc1->GetNextLine( pm->pos1 );
	pDoc2->GetNextLine( pm->pos2 );
	--pm->nLines1;
	--pm->nLines2;
	if ( bRight )
		pm->nMatch = CompareBlock( pm->aItems, pDoc2, pm->pos2, pos2U, pDoc1, pm->pos1, pos1U );
	else
		pm->nMatch = CompareBlock( pm->aItems, pDoc1, pm->pos1, pos1U, pDoc2, pm->pos2, pos2U );

	// 1: accept match
	pm = m + 1;
	pm->nLines1 = nLines1;
	pm->nLines2 = nLines2;
	pm->pos1 = pos1;
	pm->pos2 = pos2;
	if ( bRight )
	{
		InsertBlock( pm->aItems, pDoc2, pm->pos2, pos2M, pm->nLines2, pDoc1, pm->pos1, pos1, pm->nLines1 );
		pm->nMatch = CompareBlock( pm->aItems, pDoc2, pm->pos2, pos2U, pDoc1, pm->pos1, pos1U );
	}
	else
	{
		InsertBlock( pm->aItems, pDoc1, pm->pos1, pos1, pm->nLines1, pDoc2, pm->pos2, pos2M, pm->nLines2 );
		pm->nMatch = CompareBlock( pm->aItems, pDoc1, pm->pos1, pos1U, pDoc2, pm->pos2, pos2U );
	}
	// 2: select
	pm = ( m[0].nMatch > m[1].nMatch ? m : m + 1 );
	aItems.InsertAt( aItems.GetSize(), &pm->aItems );
	pos1 = pm->pos1;
	pos2 = pm->pos2;
	nLines1 = pm->nLines1;
	nLines2 = pm->nLines2;
	TRACE1( "MatchBlock matched %d\n", pm->nMatch );
	return pm->nMatch;
}

void CViewText::FindStartItem( int &nItem, POSLINE &posL, POSLINE &posR )
{
	// find position pair preceeded by unique pair starting with nItem
	posL = NULL;
	posR = NULL;
	POSLINE posL1 = NULL;
	POSLINE posR1 = NULL;
	for ( --nItem; nItem >= 0; --nItem )
	{
		posL = m_aItemData[nItem].pos[0];
		posR = m_aItemData[nItem].pos[1];
		if ( !m_aItemData[nItem].bMark ) {
			if ( GetDoc(0)->GetLineDataAt(posL).bUnique &&
				 GetDoc(1)->GetLineDataAt(posR).bUnique )
				break;
		}
		if ( posL != NULL )
			posL1 = posL;
		if ( posR != NULL )
			posR1 = posR;
	}
	if ( nItem < 0 )
	{
		nItem = 0;
		posL = posL1;		// m_aItemData[0].pos[0];
		posR = posR1;		// m_aItemData[0].pos[1];
	}
	else
	{
		++nItem;
		GetDoc(0)->GetNextLine( posL );
		GetDoc(1)->GetNextLine( posR );
	}
}

void CViewText::FindEndItem( int &nItem, POSLINE &posL, POSLINE &posR )
{
	// find unique position pair starting with nItem
	for ( ; nItem < m_aItemData.GetSize(); ++nItem )
	{
		if ( m_aItemData[nItem].bMark )
			continue;
		posL = m_aItemData[nItem].pos[0];
		posR = m_aItemData[nItem].pos[1];
		if ( posL == NULL && posR == NULL ) {	// end of list
			posL = POSLINE_END;
			posR = POSLINE_END;
			break;
		}
		if ( GetDoc(0)->GetLineDataAt(posL).bUnique &&
			 GetDoc(1)->GetLineDataAt(posR).bUnique )
			break;
	}
}

void CViewText::OnUpdateViewNextDiff(CCmdUI *pCmdUI)
{
	if ( m_pEdit != NULL )
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

void CViewText::OnViewNextDiff()
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

void CViewText::OnUpdateViewPrevDiff(CCmdUI *pCmdUI)
{
	if ( m_pEdit != NULL )
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	int nTop = m_list.GetTopIndex();
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

void CViewText::OnViewPrevDiff()
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

void CViewText::UpdateHScroll()
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
	TRACE3( "CViewText::UpdateHScroll nMax=%d nPage=%d nPos=%d\n", si.nMax, si.nPage, si.nPos );
	SetScrollInfo( SB_HORZ, &si );
}


void CViewText::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void CViewText::OnSize(UINT nType, int cx, int cy)
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
	}
	CViewFileSync::OnSize(nType, cx, cy);

	UpdateHScroll();
}

BOOL CViewText::IsAnySel()
{
	int nCount = m_list.GetSelCount();
	if ( nCount < 1 )
		return FALSE;

	int nSelAnchor = m_list.GetAnchorIndex();
	int nSelCaret = m_list.GetCaretIndex();
	if ( nSelAnchor == LB_ERR )
		nSelAnchor = nSelCaret;
	ASSERT( nSelCaret != LB_ERR );

	int nStart;
	int nEnd;
	if ( nSelAnchor < nSelCaret )
	{
		nStart = nSelAnchor;
		nEnd = nSelCaret;
	} else {
		nStart = nSelCaret;
		nEnd = nSelAnchor;
	}
	ASSERT( nStart != LB_ERR );
	ASSERT( nEnd != LB_ERR );
	if ( nEnd >= (m_aItemData.GetSize()-1) )
		--nEnd;

	int nSide = s_nSide;

	for ( int n = nStart; n <= nEnd; ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
			return TRUE;
	}
	return FALSE;
}

HGLOBAL CViewText::SelCopy()
{
	int nCount = m_list.GetSelCount();
	if ( nCount < 1 )
		return NULL;

	int nSelAnchor = m_list.GetAnchorIndex();
	int nSelCaret = m_list.GetCaretIndex();
	ASSERT( nSelAnchor != LB_ERR );
	ASSERT( nSelCaret != LB_ERR );

	// count not empty lines ...
	int n;
	int nStart;
	int nEnd;
	if ( nSelAnchor < nSelCaret )
	{
		nStart = nSelAnchor;
		nEnd = nSelCaret;
	} else {
		nStart = nSelCaret;
		nEnd = nSelAnchor;
	}
	ASSERT( nStart != LB_ERR );
	ASSERT( nEnd != LB_ERR );
	TRACE2( "CViewText::SelCopy start=%d end=%d\n", nStart, nEnd );

	int nSide = s_nSide;
	CDocText *pDoc = GetCurrDoc();

	// compute size
	int nChar = 0;
	for ( n = nStart; n <= nEnd; ++n )
	{
		POSLINE pos = m_aItemData[n].pos[nSide];
		if ( pos != NULL )
			nChar += pDoc->GetDisplayLineLen( pos ) + 2;
	}
	if ( nChar < 1 )
		return NULL;	// nothing to copy

	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, (nChar + 1) * sizeof(TCHAR) );
	if (hg == NULL) 
		return NULL; 

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hg); 
	for ( n = nStart; n <= nEnd; ++n )
	{
		POSLINE pos = m_aItemData[n].pos[nSide];
		if ( pos != NULL )
		{
			LPCTSTR pszDL = pDoc->GetDisplayLine( pos );
			int nLen = pDoc->GetDisplayLineLen( pos );
			memcpy(lptstrCopy, pszDL, nLen * sizeof(TCHAR));
			lptstrCopy += nLen;
			*lptstrCopy++ = (TCHAR)'\r';
			*lptstrCopy++ = (TCHAR)'\n';
		}
	}
	*lptstrCopy = (TCHAR) 0;    // null character 
	GlobalUnlock(hg); 

	return hg;
}

void CViewText::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsAnySel() );
}

void CViewText::OnEditCopy()
{
	if ( m_pEdit != NULL && m_pEdit->m_hWnd != NULL )
	{
		m_pEdit->Copy();
		return;
	}
	if ( !OpenClipboard() )
		return;
	if ( !EmptyClipboard() )
		return;
	HGLOBAL hg = SelCopy();
	if ( hg != NULL )
#ifdef _UNICODE
		SetClipboardData(CF_UNICODETEXT, hg);
#else
		SetClipboardData(CF_TEXT, hg);
#endif
	CloseClipboard();
}

void CViewText::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
#ifdef _UNICODE
	pCmdUI->Enable( IsClipboardFormatAvailable(CF_UNICODETEXT) && !GetCurrDoc()->IsReadOnly() );
#else
	pCmdUI->Enable( IsClipboardFormatAvailable(CF_TEXT) && !GetCurrDoc()->IsReadOnly() );
#endif
}

void CViewText::OnEditPaste()
{
	if ( m_pEdit != NULL && m_pEdit->m_hWnd != NULL )
	{
		m_pEdit->Paste();
		return;
	}
	if ( !OpenClipboard() )
		return;
#ifdef _UNICODE
	HGLOBAL hg = GetClipboardData(CF_UNICODETEXT);;
#else
	HGLOBAL hg = GetClipboardData(CF_TEXT);;
#endif
	if ( hg != NULL )
	{
		CUndoInsertText *pTask = NULL;
		if ( m_list.GetCaretIndex() != LB_ERR )
			pTask = new CUndoInsertText( this, hg );

		if ( pTask != NULL )
		{
			if ( !m_undoBuffer.AddTask( pTask ) )
				delete pTask;
		}
	}
	CloseClipboard();
	UpdateHScroll();
}

POSLINE CViewText::FindInternalLine( int nSide, int nItem )
{
	// skip empty items to find data ...
	for ( int n = nItem; n < m_aItemData.GetSize(); ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
			return m_aItemData[n].pos[nSide];
	}
	return NULL;
}

int CViewText::GetInternalLineNo( int nSide, POSLINE pos )
{
	if ( pos == NULL )
		return 0;
	return GetDoc(nSide)->GetLineNo( pos );
}

POSLINE CViewText::InsertInternalLines( int nSide, POSLINE &pos, const CStringArray &astrLines )
{
	return GetDoc(nSide)->InsertInternalLines( pos, astrLines );
}

void CViewText::FindSelectedLines( int nSide, int nItem, int nItemLast, int &nLineNoSel, int &nLinesSel )
{
	nLineNoSel = 0;
	nLinesSel = 0;
	int n;
	// skip empty items to find data ...
	for ( n = nItem; n < m_aItemData.GetSize(); ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
		{
			nLineNoSel = GetDoc(nSide)->GetLineNo( m_aItemData[n].pos[nSide] );
			break;
		}
	}
	if ( n == m_aItemData.GetSize() )
		nLineNoSel = INT_MAX;
	for ( ; n <= nItemLast; ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
			++nLinesSel;
	}
}

void CViewText::FindItemRange( int nSide, int nItem, int &nStart, int &nCount )
{
	// convert nStart from lineno to item and nCount from internal to shown
	for ( int n = nItem; n < m_aItemData.GetSize(); ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
		{
			if ( GetDoc(nSide)->GetLineNo( m_aItemData[n].pos[nSide] ) == nStart )
			{
				nStart = n;		// return start item
				int c = nCount;
				nCount = 0;		// init return item count
				for ( ; c > 0 && n < m_aItemData.GetSize(); ++n )
				{
					if ( m_aItemData[n].pos[nSide] != NULL )
					{
						--c;
						nCount = n - nStart + 1;	// return item count
					}
				}
				return;
			}
		}
	}
	// nStart not found in m_aItemData
	nStart = (int)m_aItemData.GetSize() - 1;
}

void CViewText::RemoveDocLines( int nSide, int nLineNoStart, int nCount )
{
	if ( nLineNoStart == INT_MAX && nCount > 0 )
		nLineNoStart = GetDoc(nSide)->GetLineCount() + 1 - nCount;
	TRACE3( "CViewText::RemoveDocLines side=%d line=%d count=%d\n", nSide, nLineNoStart, nCount );
	POSLINE posStart = GetDoc(nSide)->FindLineNo( nLineNoStart, NULL );
	GetDoc(nSide)->RemoveInternalLines( posStart, nCount );
}

void CViewText::InsertDocLines( int nSide, int &nLineNoStart, const CStringArray &astrLinesRaw )
{
	TRACE3( "CViewText::InsertDocLines side=%d line=%d count=%d\n", nSide, nLineNoStart, (int)astrLinesRaw.GetSize() );
	POSLINE posStart = NULL;
	if ( nLineNoStart != INT_MAX )
		posStart = GetDoc(nSide)->FindLineNo( nLineNoStart, NULL );
	else
		nLineNoStart = GetDoc(nSide)->GetLineCount() + 1;
	GetDoc(nSide)->InsertInternalLines( posStart, astrLinesRaw );
}

int CViewText::GetDocLineCount( int nSide )
{
	return GetDoc(nSide)->GetLineCount();
}

void CViewText::CopyDocLines( int nSide, int nItem, int nCount, CStringArray &astrLinesRaw )
{
	for ( int n = nItem; nCount > 0 && n < m_aItemData.GetSize(); --nCount, ++n )
	{
		if ( m_aItemData[n].pos[nSide] != NULL )
		{
			astrLinesRaw.Add( GetDoc(nSide)->GetRawLine( m_aItemData[n].pos[nSide] ) );
		}
	}
}

void CViewText::OnUpdateEditCut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsAnySel() && !GetCurrDoc()->IsReadOnly() );
}

void CViewText::OnEditCut()
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->Cut();
		return;
	}
	OnEditCopy();
	OnEditDelete();
}

void CViewText::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_undoBuffer.CanUndo() && m_pEdit == NULL );
}

void CViewText::OnEditUndo()
{
	m_undoBuffer.Undo();
}

void CViewText::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_undoBuffer.CanRedo() && m_pEdit == NULL );
}

void CViewText::OnEditRedo()
{
	m_undoBuffer.Redo();
}

void CViewText::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	if ( m_pEdit == NULL || m_pEdit->m_hWnd == NULL)
		pCmdUI->Enable( IsAnySel() && !GetCurrDoc()->IsReadOnly() );
	else
		pCmdUI->Enable(FALSE);
}

void CViewText::OnEditDelete()
{
	CUndoDelete *pTask = NULL;
	pTask = new CUndoDelete( this );
	if ( pTask == NULL )
		return;

	// pTask->m_bKeepSel = TRUE;		// TODO test only
	if ( !m_undoBuffer.AddTask( pTask ) )
		delete pTask;
}

void CViewText::OnUpdateEditDel(CCmdUI *pCmdUI)
{
	if ( m_pEdit != NULL && m_pEdit->m_hWnd != NULL)
		pCmdUI->Enable();
}

void CViewText::OnEditDel()
{
	if ( m_pEdit != NULL && m_pEdit->m_hWnd != NULL )
	{
		m_pEdit->OnCharDelete();
		return;
	}
}

void CViewText::OnUpdateEditInsertafter(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsAnySel() && !GetOtherDoc()->IsReadOnly() && m_pEdit == NULL );
}

void CViewText::OnEditInsertafter()
{
	CUndoInsertAfter *pTask = NULL;
	pTask = new CUndoInsertAfter( this );
	if ( pTask == NULL )
		return;

	if ( !m_undoBuffer.AddTask( pTask ) )
		delete pTask;
}

void CViewText::OnUpdateEditInsertbefore(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsAnySel() && !GetOtherDoc()->IsReadOnly() && m_pEdit == NULL );
}

void CViewText::OnEditInsertbefore()
{
	CUndoInsertBefore *pTask = NULL;
	pTask = new CUndoInsertBefore( this );
	if ( pTask == NULL )
		return;

	if ( !m_undoBuffer.AddTask( pTask ) )
		delete pTask;
}

void CViewText::OnUpdateEditReplacesel(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsAnySel() && !GetOtherDoc()->IsReadOnly() && m_pEdit == NULL );
}

void CViewText::OnEditReplacesel()
{
	CUndoReplace *pTask = NULL;
	pTask = new CUndoReplace( this );
	if ( pTask == NULL )
		return;

	if ( !m_undoBuffer.AddTask( pTask ) )
		delete pTask;
}

void CViewText::OnEditPref()
{
	CPrefDlg dlg;
	dlg.m_nTabSize = GetCurrDoc()->GetTabSize();
	dlg.m_bIgnSpaces = GetCurrDoc()->IsCompactSpace();
	dlg.m_bUnixLeft = GetDoc(0)->IsUnixFormat();
	dlg.m_bUnixRight = GetDoc(1)->IsUnixFormat();
	dlg.m_bReadOnlyLeft = GetDoc(0)->IsReadOnly();
	dlg.m_bReadOnlyRight = GetDoc(1)->IsReadOnly();
	dlg.m_bFileROLeft = GetDoc(0)->IsReadOnlyFile();
	dlg.m_bFileRORight = GetDoc(1)->IsReadOnlyFile();
	dlg.m_strEncodingL = GetDoc(0)->GetEncoding();
	dlg.m_strEncodingR = GetDoc(1)->GetEncoding();
	int nTabSize = dlg.m_nTabSize;
	BOOL bIgnSpaces = dlg.m_bIgnSpaces;

	INT_PTR nRc = dlg.DoModal();
	if ( nRc == IDOK )
	{
		GetDoc(0)->SetTabSize( dlg.m_nTabSize );
		GetDoc(1)->SetTabSize( dlg.m_nTabSize );
		GetDoc(0)->SetCompactSpace( dlg.m_bIgnSpaces );
		GetDoc(1)->SetCompactSpace( dlg.m_bIgnSpaces );
		if ( GetDoc(0)->IsUnixFormat() != dlg.m_bUnixLeft )
		{
			GetDoc(0)->SetUnixFormat( dlg.m_bUnixLeft );
			GetDoc(0)->SetModifiedFlag();
		}
		if ( GetDoc(1)->IsUnixFormat() != dlg.m_bUnixRight )
		{
			GetDoc(1)->SetUnixFormat( dlg.m_bUnixRight );
			GetDoc(1)->SetModifiedFlag();
		}
		GetDoc(0)->SetReadOnly( dlg.m_bReadOnlyLeft );
		GetDoc(1)->SetReadOnly( dlg.m_bReadOnlyRight );
		GetDoc(0)->SetReadOnlyFile(dlg.m_bFileROLeft);
		GetDoc(1)->SetReadOnlyFile(dlg.m_bFileRORight);
		if ( nTabSize != dlg.m_nTabSize || 
			 bIgnSpaces != dlg.m_bIgnSpaces )
		{
			if ( nTabSize != dlg.m_nTabSize || 
				bIgnSpaces != dlg.m_bIgnSpaces )
			{
				GetDoc(0)->UpdateInternalLines();
				GetDoc(1)->UpdateInternalLines();
			}
			CompareView();
			SetScrollPos( SB_HORZ, 0, FALSE );
			UpdateHScroll();
			m_undoBuffer.RemoveAll();
		}
		if ( GetDoc(0)->GetEncoding() != dlg.m_strEncodingL )
		{
			GetDoc(0)->SetEncoding( dlg.m_strEncodingL );
			GetDoc(0)->SetModifiedFlag();
		}
		if ( GetDoc(1)->GetEncoding() != dlg.m_strEncodingR )
		{
			GetDoc(1)->SetEncoding( dlg.m_strEncodingR );
			GetDoc(1)->SetModifiedFlag();
		}
	}
}

int CViewText::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_strColorsSection = "Text";
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

BOOL CViewText::IsNoDifference()
{
	if ( GetDoc(0) == NULL || GetDoc(0)->IsModified() ||
		 GetDoc(1) == NULL || GetDoc(1)->IsModified() )
		return FALSE;

	for ( int n = 0; n < m_list.GetCount(); ++n )
	{
		if ( m_aItemData[n].bMark )
			return FALSE;
	}
	return TRUE;
}

void CViewText::OnListLButtonDown()
{
	if ( m_pEdit != NULL )
	{
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
	}
	SetFocus();
}

void CViewText::OnListDblClk(UINT nItem, int nSide, const CRect &rect, CPoint point)
{
	if ( GetKeyState(VK_CONTROL) & 0x8000 ) {
		QuickFix( nItem, nSide, rect, point );
		return;
	}
//	if (m_pDoc[nSide]->IsReadOnly()) {
//		Beep(100,100);
//		return;
//	}
	m_list.SelItemRange( FALSE, 0, m_list.GetCount()-1 );
	m_list.SetSel( nItem );
	m_pEdit = new CEditLine();
	CRect rectEdit = rect;
	rectEdit.left += m_nWidthLineNo;
	rectEdit.bottom++;
	rectEdit.top -= 3;
	m_pEdit->Create( WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rectEdit, &m_list, IDC_EDITLINE );
	m_pEdit->SetFont( &m_fontList );
	CDocText *pDoc = GetDoc(nSide);
	POSLINE pos = m_aItemData[nItem].pos[nSide];
	LPCTSTR lpszText = pDoc->GetDisplayLine( pos );
	m_pEdit->SetWindowText( lpszText );
	int nStart = m_pEdit->CharFromPos( CPoint(point.x-m_nWidthLineNo, point.y) );
	m_pEdit->SetFocus();
	m_pEdit->SetSel( nStart, nStart, 0 );
}

void CViewText::OnEditLineEnter()
{
	ASSERT( m_pEdit != NULL );

	if (GetDoc(s_nSide)->IsReadOnly()) {
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
		//Beep(100,100);
		MessageBeep(MB_ICONWARNING);
		return;
	}

	int nStart = -1;
	int nEnd = -1;
	BOOL bCombine = FALSE;
	if ( GetKeyState(VK_SHIFT) & 0x8000 ) {		// get cursor pos for splitting line
		m_pEdit->GetSel( nStart, nEnd );
	}
	else if ( GetKeyState(VK_CONTROL) & 0x8000 ) {		// prepare combine
		int nSelAnchor = m_list.GetAnchorIndex();
		if ( nSelAnchor != LB_ERR && (nSelAnchor+2) < m_list.GetCount()) {
			bCombine = TRUE;
			m_list.SelItemRange(TRUE, nSelAnchor, nSelAnchor+1);
		}
	}

	CString strText;
	m_pEdit->GetWindowText( strText );
	if ( bCombine ) {
		// add 2nd line
		CDocText *pDoc = GetDoc(s_nSide);
		int nItem = m_list.GetSelFirst();
		if ( m_aItemData.GetSize() > (nItem+1) ) {
			POSLINE pos = m_aItemData[nItem+1].pos[s_nSide];
			LPCTSTR lpszText = pDoc->GetDisplayLine( pos );
			strText += lpszText;
		}
		else
			bCombine = FALSE;	// 2nd line not found
	}
	int nChar = strText.GetLength();
	HGLOBAL hg = NULL;
	if ( nChar > 0 )
	{
		int nPlus = 3;
		if ( nStart >= 0 )
			nPlus = 5;
		hg = GlobalAlloc(GMEM_MOVEABLE, (nChar + nPlus) * sizeof(TCHAR) );
		if (hg == NULL) 
			AfxThrowMemoryException(); 

		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hg); 
		LPCTSTR lptstrSrc = (LPCTSTR)strText;
		if ( nStart >= 0 && nStart <= nChar ) {
			memcpy(lptstrCopy, lptstrSrc, nStart * sizeof(TCHAR));
			lptstrCopy += nStart;
			lptstrSrc += nStart;
			nChar -= nStart;
			*lptstrCopy++ = (TCHAR)'\r';
			*lptstrCopy++ = (TCHAR)'\n';
		}
		memcpy(lptstrCopy, lptstrSrc, nChar * sizeof(TCHAR));
		lptstrCopy += nChar;
		*lptstrCopy++ = (TCHAR)'\r';
		*lptstrCopy++ = (TCHAR)'\n';
		*lptstrCopy = (TCHAR) 0;    // null character 
		GlobalUnlock(hg); 
	}

	CUndoReplaceText *pTask = NULL;
	if ( m_list.GetCaretIndex() != LB_ERR )
		pTask = new CUndoReplaceText( this, hg );

	if ( pTask != NULL )
	{
		if ( !m_undoBuffer.AddTask( pTask ) )
			delete pTask;
	}

	m_pEdit->DestroyWindow();
	delete m_pEdit;
	m_pEdit = NULL;
	UpdateHScroll();
}

void CViewText::QuickFix(UINT nItem, int nSide, const CRect &rect, CPoint point)
{
	if (m_pDoc[1-nSide]->IsReadOnly()) {
		//Beep(100,100);	// other side is RO
		MessageBeep(MB_ICONWARNING);
		return;
	}
	if ( point.x < m_nWidthLineNo )
		QuickFixBlock( nItem, nSide, rect, point );
	else
		QuickFixLine( nItem, nSide, rect, point );
}

void CViewText::QuickFixBlock(UINT nItem, int nSide, const CRect &rect, CPoint point)
{
	if ( GetOtherDoc()->IsReadOnly() )
		return;	// not allowed

	BOOL bMark = m_aItemData[nItem].bMark;
	if ( !bMark )
		return;	// no diff

	UINT nItemStart = nItem;
	UINT nItemEnd = nItem;
	while ( nItemStart > 0 && m_aItemData[nItemStart].bMark == bMark ) {
		--nItemStart; 
	}
	if ( m_aItemData[nItemStart].bMark != bMark )
		++nItemStart;
	while ( (int)nItemEnd < m_aItemData.GetSize()-1 && m_aItemData[nItemEnd].bMark == bMark ) {
		++nItemEnd; 
	}
	if ( m_aItemData[nItemEnd].bMark != bMark )
		--nItemEnd;
	m_list.SetAnchorIndex( nItemStart );
	m_list.SelItemRange( TRUE, nItemStart, nItemEnd );
	OnEditReplacesel();
}

void CViewText::QuickFixLine(UINT nItem, int nSide, const CRect &rect, CPoint point)
{
	POSLINE pos = m_aItemData[nItem].pos[nSide];
	CDocText *pDoc = GetDoc(nSide);
//	int nLineNo = pDoc->GetLineNo( pos );
	LPCTSTR lpszText = pDoc->GetDisplayLine( pos );
	int nLen = pDoc->GetDisplayLineLen( pos );
	LPCTSTR pszChar = lpszText + m_nCharOffs;
	CStringDiff &sd = pDoc->GetStringDiff( pos );
	nLen -= m_nCharOffs;
	int nCharWidth = m_list.GetCharWidth();
	int p = m_nCharOffs + (point.x-m_nWidthLineNo)/nCharWidth;
	if ( p < nLen )
	{
		CString strText = sd.Merge( pDoc->GetDisplayLine( pos ), p, 
			GetDoc(1-nSide)->GetDisplayLine( m_aItemData[nItem].pos[1-nSide] ) );
		TRACE1( "  M=%s\n", strText );

		int nChar = strText.GetLength();
		HGLOBAL hg = NULL;
		if ( nChar > 0 )
		{
			int nPlus = 3;
			hg = GlobalAlloc(GMEM_MOVEABLE, (nChar + nPlus) * sizeof(TCHAR) );
			if (hg == NULL) 
				AfxThrowMemoryException(); 

			LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hg); 
			LPCTSTR lptstrSrc = (LPCTSTR)strText;
			memcpy(lptstrCopy, lptstrSrc, nChar * sizeof(TCHAR));
			lptstrCopy += nChar;
			*lptstrCopy++ = (TCHAR)'\r';
			*lptstrCopy++ = (TCHAR)'\n';
			*lptstrCopy = (TCHAR) 0;    // null character 
			GlobalUnlock(hg); 
		}
		else
			//Beep(100,100);
			MessageBeep(MB_ICONWARNING);

		CUndoReplaceText *pTask = NULL;
		if ( m_list.GetCaretIndex() != LB_ERR )
			pTask = new CUndoReplaceText( this, hg );

		if ( pTask != NULL )
		{
			if ( nSide == left )	// sel other side
				SetSide( right );
			else
				SetSide( left );
			if ( !m_undoBuffer.AddTask( pTask ) )
				delete pTask;
			if ( nSide == right )	// reset
				SetSide( right );
			else
				SetSide( left );
			UpdateHScroll();
		}
	}
}

void CViewText::OnListVScroll()
{
	OnListLButtonDown();	// remove m_pEdit if present
}

void CViewText::OnSearch()
{
	OnSearchF();
}

void CViewText::OnSearchB()
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

	CDocText *pDoc = GetCurrDoc();
	for ( int nItem = nStart; nItem >= 0; --nItem )
	{
		CString str;
		POSLINE pos = m_aItemData[nItem].pos[s_nSide];
		if ( pos != NULL )
		{
			str = pDoc->GetDisplayLine( pos );
			str = str.MakeLower();
		}
//		TRACE1("B %s\n", str);
		if ( str.Find( strSearch ) >= 0 ) {
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
	int nItem = m_list.GetSelFirst();
	if ( nItem >= 0 && ( nItem < m_list.GetTopIndex() || nItem > m_list.GetBottomIndex() ) )
		m_list.SetTopIndex( nItem );
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

void CViewText::OnSearchF()
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

	CDocText *pDoc = GetCurrDoc();
	for ( int nItem = nStart; nItem < nEnd; ++nItem )
	{
		CString str;
		POSLINE pos = m_aItemData[nItem].pos[s_nSide];
		if ( pos != NULL )
		{
			str = pDoc->GetDisplayLine( pos );
			str = str.MakeLower();
		}
//		TRACE1("F %s\n", str);
		if ( str.Find( strSearch ) >= 0 ) {
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
	int nItem = m_list.GetSelFirst();
	if ( nItem >= 0 && ( nItem < m_list.GetTopIndex() || nItem > m_list.GetBottomIndex() ) )
		m_list.SetTopIndex( nItem );
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

void CViewText::OnDeactivateApp()
{
	CViewFileSync::OnDeactivateApp();
	if ( m_pEdit != NULL )
	{
		m_pEdit->DestroyWindow();
		delete m_pEdit;
		m_pEdit = NULL;
	}
}


void CViewText::OnHelpContext()
{
	CFileSyncApp* pApp = (CFileSyncApp*)AfxGetApp();
	ShellExecute( GetParentFrame()->m_hWnd, _T("open"), _T("viewtext.html"), NULL, pApp->m_strHelpPath, SW_SHOW );
}


void CViewText::OnUpdateViewText(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( FALSE );
}


BOOL CViewText::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_MOUSEWHEEL)
    {
		m_list.SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
		return TRUE;
	}
	return CViewList::PreTranslateMessage( pMsg );
}
