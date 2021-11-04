#include "StdAfx.h"
//#include "ViewFileSync.h"

#include "UndoTaskFileSync.h"

CUndoTaskFileSync::CUndoTaskFileSync(CViewList *pView)
{
	m_pView = pView;

	m_nSide = CViewFileSync::left;
	m_nTopIndex = 0;
}

CUndoTaskFileSync::~CUndoTaskFileSync(void)
{
}

BOOL CUndoTaskFileSync::DoFirst(void)
{
	TRACE0( "CUndoTaskFileSync::DoFirst\n" );
	if ( !PrepareRange() )
		return FALSE;

	return TRUE;
}

BOOL CUndoTaskFileSync::PrepareRange(void)
{
	m_nSide = m_pView->GetSide();
	CDualListBox& dlb = GetLB();
	m_nTopIndex = dlb.GetTopIndex();
	m_nSelAnchor = dlb.GetAnchorIndex();
	if ( m_nSelAnchor == LB_ERR )
		m_nSelAnchor = dlb.GetCaretIndex();
	ASSERT( m_nSelAnchor != LB_ERR );
	m_nSelCount = dlb.GetSelCount();
	TRACE2( "CUndoTaskFileSync::PrepareRange anchor=%d count=%d\n", m_nSelAnchor, m_nSelCount );
	if ( m_nSelCount > 1 )
	{
		int nSelCaret = dlb.GetCaretIndex();
		ASSERT( nSelCaret != LB_ERR );
		if ( m_nSelAnchor > nSelCaret )
			m_nSelAnchor = nSelCaret;
	}
	// deselect last line of listbox
	if ( (m_nSelAnchor + m_nSelCount) > m_pView->m_list.GetCount() )	// 091203 before: >=
		--m_nSelCount;
	return TRUE;
}

void CUndoTaskFileSync::SetSelection( CViewFileSync::Side nSide, int nStart, int nCount )
{
	TRACE3( "CUndoTaskFileSync::SetSelection side=%d start=%d count=%d\n", nSide, nStart, nCount );

	CDualListBox& dlb = GetLB();
	dlb.SetTopIndex( m_nTopIndex );
	dlb.SetSel( -1, FALSE );
	dlb.SetAnchorIndex( nStart );	// ????
	dlb.SetCaretIndex( nStart );
	if ( nCount > 1 )
	{
		dlb.SelItemRange( TRUE, nStart, nStart+nCount-1 );
		dlb.SetCaretIndex( nStart+nCount-1 );		// PP 050331
	}
	else if ( nCount == 1 )
		dlb.SetSel( nStart );

	m_pView->SelectSide( nSide );
	dlb.Invalidate();
}

