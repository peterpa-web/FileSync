#include "StdAfx.h"
#include "DocText.h"
#include "UndoDelete.h"

CUndoDelete::CUndoDelete(CViewText *pView) :
		CUndoViewText(pView)
{
	m_bKeepSel = FALSE;
}

CUndoDelete::~CUndoDelete(void)
{
}

BOOL CUndoDelete::DoFirst()		// 2011/06/24
{
	TRACE0( "CUndoDelete::DoFirst\n" );
	if ( !CUndoViewText::DoFirst() )
		return FALSE;

	if ( m_nLineNoStart[m_nSide] == 0 && (GetDoc(m_nSide)->GetLineCount() > m_nLinesSel ) )	{	
		TRACE0( "CUndoDelete::DoFirst adj LineNoStart own\n" );
		m_nLineNoStart[m_nSide] = 1;
	}
	if ( m_nLineNoStart[OtherSide()] == 0 && (GetDoc(OtherSide())->GetLineCount() > 0 )) {
		TRACE0( "CUndoDelete::DoFirst adj LineNoStart other\n" );
		m_nLineNoStart[OtherSide()] = 1;
	}
	return TRUE;
}

BOOL CUndoDelete::Do()
{
	if ( m_nLinesSel < 1 )		// 091203 nothing to delete
		return FALSE;

	TRACE0( "CUndoDelete::Do\n" );

	GetView()->RemoveDocLines( m_nSide, m_nStartInternalLineNo, (int)m_astrRawLines.GetSize() );
//	if ( m_nLineNoStart[m_nSide] == 0 )			091203
//		m_nLineNoStart[m_nSide] = 1;
//	if ( m_nLineNoStart[OtherSide()] == 0 )
//		m_nLineNoStart[OtherSide()] = 1;

	UpdateView( m_nSide, - (int)m_astrRawLines.GetSize() );
	SetSelFromLineNo( m_nSide, m_nStartInternalLineNo, 1 );

	return TRUE;
}

BOOL CUndoDelete::Undo()
{
	TRACE0( "CUndoDelete::Undo\n" );

	GetView()->InsertDocLines( m_nSide, m_nStartInternalLineNo, m_astrRawLines );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}
