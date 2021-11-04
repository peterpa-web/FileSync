#include "StdAfx.h"
#include "UndoTaskFileSync.h"
#include "DocText.h"

#include "UndoViewText.h"

CUndoViewText::CUndoViewText(CViewText *pView) :
				CUndoTaskFileSync( pView )
{
	m_nLineNoStart[0] = 0;
	m_nLineNoStart[1] = 0;
	m_nLineNoEnd[0] = 0;
	m_nLineNoEnd[1] = 0;
	m_nStartSave = 0;
	m_nInsertedItems = 0;
	m_nLineNoSel = 0;
	m_nLinesSel = 0;
	m_nStartInternalLineNo = 0;
	m_nLineNoSelOther = 0;
	m_nLinesSelOther = 0;
	m_nStartInternalLineNoOther = 0;
}

CUndoViewText::~CUndoViewText(void)
{
}

BOOL CUndoViewText::DoFirst(void)
{
	TRACE0( "CUndoViewText::DoFirst\n" );
	if ( !CUndoTaskFileSync::DoFirst() )
		return FALSE;
	if ( !SaveLines() )
		return FALSE;

	GetView()->CopyDocLines( m_nSide, m_nSelAnchor, m_nSelCount, m_astrRawLines );
	return TRUE;
}

BOOL CUndoViewText::SaveLines(void)
{
	GetView()->FindSelectedLines( m_nSide, m_nSelAnchor, m_nSelAnchor + m_nSelCount - 1, 
									m_nLineNoSel, m_nLinesSel );
	m_nStartInternalLineNo = m_nLineNoSel;
	TRACE2( "CUndoViewText::SaveLines own   from %d count %d\n", m_nLineNoSel, m_nLinesSel );
//	if ( m_nStartInternalLineNo == 0 )
//		m_nStartInternalLineNo = 1;

	GetView()->FindSelectedLines( OtherSide(), m_nSelAnchor, m_nSelAnchor + m_nSelCount - 1, 
									m_nLineNoSelOther, m_nLinesSelOther );
	m_nStartInternalLineNoOther = m_nLineNoSelOther;
	TRACE2( "CUndoViewText::SaveLines other from %d count %d\n", m_nLineNoSelOther, m_nLinesSelOther );
//	if ( m_nStartInternalLineNoOther == 0 )
//		m_nStartInternalLineNoOther = 1;

	return SaveItemData( m_nSelAnchor, m_nSelCount );
}

BOOL CUndoViewText::SaveItemData( int nStart, int nCount )
{
	m_bDocModified[0] = GetDoc(0)->IsModified();
	m_bDocModified[1] = GetDoc(1)->IsModified();

	TRACE2( "CUndoViewText::SaveItemData start=%d count=%d\n", nStart, nCount );
	POSLINE posL;
	POSLINE posR;
	m_nStartSave = nStart;
	GetView()->FindStartItem( m_nStartSave, posL, posR );
	m_nLineNoStart[0] = GetDoc(0)->GetLineNo(posL);
	m_nLineNoStart[1] = GetDoc(1)->GetLineNo(posR);

	int nEndSave = nStart + nCount;
	GetView()->FindEndItem( nEndSave, posL, posR );
	m_nLineNoEnd[0] = GetDoc(0)->GetLineNo(posL);
	m_nLineNoEnd[1] = GetDoc(1)->GetLineNo(posR);

	if ( m_nLineNoEnd[0] != 0 && m_nLineNoStart[0] == 0 )	// 060602
		m_nLineNoStart[0] = 1;
	if ( m_nLineNoEnd[1] != 0 && m_nLineNoStart[1] == 0 )
		m_nLineNoStart[1] = 1;

	TRACE2( "  start %d %d\n", m_nLineNoStart[0], m_nLineNoStart[1] );
	TRACE2( "  end   %d %d\n", m_nLineNoEnd[0], m_nLineNoEnd[1] );

//	int line[2];
//	line[0] = m_nLineNoStart[0];			// DEBUG
//	line[1] = m_nLineNoStart[1];
//-	int nLines = 0;
	int n = m_nStartSave;
	nCount = nEndSave - m_nStartSave;
	TRACE2( "  saving n=%d count=%d\n", n, nCount );
	for ( int i = 0; i < nCount; ++n, ++i )
	{
		ViewItem vi;
		for ( int s = 0; s < 2; ++s )
		{
			POSLINE pos = GetView()->m_aItemData[n].pos[s];
			vi.nLineNo[s] = ( pos == NULL ? 0 : GetDoc(s)->GetLineNo(pos) );
//-			if ( s == m_nSide && pos != NULL )
//-				++nLines;
//			if ( pos != NULL )							// DEBUG
//				ASSERT( line[s]++ == vi.nLineNo[s] );
		}
		vi.bMark = GetView()->m_aItemData[n].bMark;
		m_aViewItems.Add(vi);
	}
//	if ( nLines < 1 )
//		return FALSE;	// nothing to do

	return TRUE;
}

void CUndoViewText::UpdateView( CViewFileSync::Side nSide, int nLinesIns )
{
	TRACE2( "CUndoViewText::UpdateView %d %d\n", m_nLineNoStart[0], m_nLineNoStart[1] );
	GetView()->DeleteItems( m_nStartSave, (int)m_aViewItems.GetSize() );

	POSLINE posL = NULL;
	if ( m_nLineNoStart[0] == INT_MAX ) {
		int nLine = GetDoc(0)->GetLineCount() + 1;
		if ( nSide == CViewFileSync::left )
			nLine -= nLinesIns;
		posL = GetDoc(0)->FindLineNo( nLine, NULL );
	}
	else
		posL = GetDoc(0)->FindLineNo( m_nLineNoStart[0], NULL );
	POSLINE posR = NULL;
	if ( m_nLineNoStart[1] == INT_MAX ) {
		int nLine = GetDoc(1)->GetLineCount() + 1;
		if ( nSide == CViewFileSync::right )
			nLine -= nLinesIns;
		posR = GetDoc(1)->FindLineNo( nLine, NULL );
	}
	else
		posR = GetDoc(1)->FindLineNo( m_nLineNoStart[1], NULL );
	TRACE2( "  UpdateView start %d %d", GetDoc(0)->GetLineNo(posL), GetDoc(1)->GetLineNo(posR) );
	int nLineNoEnd[2];
	nLineNoEnd[0] = m_nLineNoEnd[0];
	nLineNoEnd[1] = m_nLineNoEnd[1];
	//* 2011-12-20
	if ( nSide == CViewFileSync::left )
	{
		if ( nLineNoEnd[0] == INT_MAX )
			nLineNoEnd[0] = GetDoc(0)->GetLineCount() + 1;
		else
			nLineNoEnd[0] += nLinesIns;
	}
	if ( nSide == CViewFileSync::right )
	{
		if ( nLineNoEnd[1] == INT_MAX )
			nLineNoEnd[1] = GetDoc(1)->GetLineCount() + 1;
		else
			nLineNoEnd[1] += nLinesIns;
	}
	//*/
	TRACE2( " - end %d %d\n", nLineNoEnd[0], nLineNoEnd[1] );
	POSLINE posLM = GetDoc(0)->FindLineNo( nLineNoEnd[0], NULL );
	POSLINE posRM = GetDoc(1)->FindLineNo( nLineNoEnd[1], NULL );

	CViewText::CItemDataArray aItems;
	BOOL b = GetView()->CompareUniqueBlock1( aItems, GetDoc(0), posL, posLM, GetDoc(1), posR, posRM );
	ASSERT( b );
	GetView()->InsertItems( m_nStartSave, &aItems );
	m_nInsertedItems = (int)aItems.GetSize();
}

void CUndoViewText::SetSelFromLineNo( CViewFileSync::Side nSide, int nLineNo, int nCount )
{
	GetView()->FindItemRange( nSide, m_nStartSave, nLineNo, nCount );
	SetSelection( nSide, nLineNo, nCount );
}

void CUndoViewText::RestoreItemData()
{
	TRACE0( "CUndoViewText::RestoreItemData\n" );
	TRACE2( "  DeleteItems start=%d count=%d\n", m_nStartSave, m_nInsertedItems );

	// delete items ...
	GetView()->DeleteItems( m_nStartSave, m_nInsertedItems );

	// insert items ...
	int nCount = (int)m_aViewItems.GetSize();
	TRACE1( "  count=%d\n", nCount );
//	if ( nCount < 1 )
//		return;

	POSLINE pos[2];
	for ( int s = 0; s < 2; ++s )
	{
		pos[s] = GetDoc(s)->FindLineNo( m_nLineNoStart[s], NULL );
	}
	int nItem = m_nStartSave;
	for ( int i = 0; i < nCount; ++i )
	{
		CViewText::ItemData d;
		for ( int s = 0; s < 2; ++s )
		{
			int nLineNo = m_aViewItems[i].nLineNo[s];
			POSLINE p = NULL;
			if ( nLineNo != 0 )
			{
				if ( !POSLINE_ISVALID(pos[s]) )
					pos[s] = GetDoc(s)->FindLineNo( nLineNo, NULL );
//				TRACE3( "RestoreItemData line s=%d requ=%d is=%d\n", s, nLineNo, GetDoc(s)->GetLineNo(pos[s]) );
				ASSERT( nLineNo == GetDoc(s)->GetLineNo(pos[s]) );
				p = pos[s];
				GetDoc(s)->GetNextLine(pos[s]);
			}
			d.pos[s] = p;
		}
		d.bMark = m_aViewItems[i].bMark;
		if ( d.pos[0] != NULL )
			GetDoc(0)->UpdateStringDiff( d.pos[0], GetDoc(1), d.pos[1] );
		else
			GetDoc(1)->UpdateStringDiff( d.pos[1], GetDoc(0), d.pos[0] );
		GetView()->InsertItem( nItem++, d );
	}
	// todo: check if doc saved after ...
	GetDoc(0)->SetModifiedFlag( m_bDocModified[0] );
	GetDoc(1)->SetModifiedFlag( m_bDocModified[1] );
}

INT_PTR CUndoViewText::GetSize()
{
	return 100 + 
			m_aViewItems.GetSize() * sizeof(ViewItem) +
			m_astrRawLines.GetSize() * 80;
}



