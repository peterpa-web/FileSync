#include "StdAfx.h"
#include "LinesListBox.h"
#include "undoinsertafter.h"

CUndoInsertAfter::CUndoInsertAfter(CLinesListBox *pLbOwn, CLinesListBox *pLbOther, CWnd* pWndScroll) :
		CUndoTaskFileSync(pLbOwn, pLbOther, pWndScroll)
{
}

CUndoInsertAfter::~CUndoInsertAfter(void)
{
}

BOOL CUndoInsertAfter::Do(void)
{
	TRACE2( "CUndoInsertAfter::Do anchor=%d caret=%d\n", m_nSelAnchor, m_nSelCaret );
	ASSERT ( m_nSelAnchor <= m_nSelCaret );

	int nStart = m_nSelAnchor;
	int nEnd = m_nSelCaret;
	int n;

	SaveLinesMarkOther();

	// copy to other ...
	int nOther = nEnd + 1;
	m_nStartInternalLines = m_pLbOther->FindInternalLine( nOther );
	int nIdx = m_nStartInternalLines;
	int nInserted = 0;
	for ( n = nStart; n <= nEnd; ++n )
	{
		DWORD dwDataOwn = (DWORD)m_pLbOwn->GetItemData( n );
		ASSERT( dwDataOwn != LB_ERR );
		if ( dwDataOwn == MARK_ITEM )
		{	// move empty line before selection
			m_pLbOwn->DeleteString( n );
			m_pLbOwn->SetStartItem( nStart );
			m_pLbOwn->InsertEmptyItem();
			continue;
		}
		m_pLbOther->InsertItem( m_pLbOwn, n, nOther++, nIdx++ );
		m_pLbOwn->SetStartItem( nStart );
		m_pLbOwn->InsertEmptyItem();
		++n;
		++nEnd;
		m_pLbOwn->SetItemData( n, dwDataOwn & MARK_ITEM_MASK );
		++nInserted;
	}

	m_nInsertedLines = nInserted;
	nEnd = m_nSelCaret;

	// delete common empty lines
	m_nDeletedLines = 0;
	for ( n = nEnd; n >= nStart; --n )
	{
		DWORD dwDataOwn = (DWORD)m_pLbOwn->GetItemData( n );
		ASSERT( dwDataOwn != LB_ERR );
		DWORD dwDataOther = (DWORD)m_pLbOther->GetItemData( n );
		ASSERT( dwDataOther != LB_ERR );
		if ( dwDataOwn == MARK_ITEM && dwDataOther == MARK_ITEM )
		{
			m_pLbOwn->DeleteString( n );
			m_pLbOther->DeleteString( n );
			++m_nDeletedLines;
		}
	}

	m_pLbOther->UpdateItemData( nEnd + nInserted - m_nDeletedLines + 1, nInserted );

	m_pLbOther->SetSel( -1, FALSE );
	m_pLbOwn->SetSel( -1, FALSE );
	m_pLbOwn->SetAnchorIndex( nStart );
	if ( nInserted > 1 )
		m_pLbOwn->SelItemRange( TRUE, nEnd - m_nDeletedLines + 1, nEnd - m_nDeletedLines + nInserted );
		m_pLbOwn->SelItemRange( TRUE, nStart, nStart + nInserted - 1 );
	else
		m_pLbOwn->SetSel( nEnd - m_nDeletedLines + 1 );

	m_pLbOwn->SetTopIndex( m_nScrollPos );
	m_pLbOther->SetTopIndex( m_nScrollPos );
	m_pWndScroll->SetScrollRange( SB_VERT, 0, m_pLbOwn->GetCount() );
	m_pWndScroll->SetScrollPos( SB_VERT, m_nScrollPos );

	return TRUE;
}

BOOL CUndoInsertAfter::Undo( BOOL bFirst )
{
	TRACE2( "CUndoInsertAfter::Undo anchor=%d caret=%d\n", m_nSelAnchor, m_nSelCaret );
	ASSERT ( m_nSelAnchor <= m_nSelCaret );

	int nStart = m_nSelAnchor;
	int nEnd = m_nSelCaret;
	int nInserted = m_nInsertedLines - m_nDeletedLines;
	ASSERT( nInserted >= 0);
	int n;

	// delete extra items
	for ( n = 0; n < nInserted; ++n )
	{
		m_pLbOwn->DeleteString( nStart );
		m_pLbOther->DeleteString( nStart );
	}

	m_pLbOther->RemoveInternalLines( m_nStartInternalLines, m_nInsertedLines );
	m_pLbOther->UpdateItemData( nStart + m_nInsertedLines, -m_nInsertedLines );

	// restore item data ...
	for ( n = 0; n <= (nEnd - nStart); ++n )
	{
		m_pLbOwn->SetItemData( nStart + n, m_adwLbDataOwn[n] );
		m_pLbOther->SetItemData( nStart + n, m_adwLbDataOther[n] );
	}

	m_pLbOther->SetSel( -1, FALSE );
	m_pLbOwn->SetSel( -1, FALSE );
	m_pLbOwn->SetAnchorIndex( nStart );
	if ( nEnd > nStart )
		m_pLbOwn->SelItemRange( TRUE, nStart, nEnd );
	else
		m_pLbOwn->SetSel( nStart );

	m_pLbOwn->SetTopIndex( m_nScrollPos );
	m_pLbOther->SetTopIndex( m_nScrollPos );
	m_pWndScroll->SetScrollRange( SB_VERT, 0, m_pLbOwn->GetCount() );
	m_pWndScroll->SetScrollPos( SB_VERT, m_nScrollPos );

	if ( bFirst )
		m_pLbOther->SetDataChanged( FALSE );

	return TRUE;
}
