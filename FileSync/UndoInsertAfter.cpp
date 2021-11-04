#include "StdAfx.h"
#include "UndoInsertAfter.h"

CUndoInsertAfter::CUndoInsertAfter(CViewText *pView) :
		CUndoViewText(pView)
{
}

CUndoInsertAfter::~CUndoInsertAfter(void)
{
}

BOOL CUndoInsertAfter::Do()
{
	TRACE0( "CUndoInsertAfter::Do\n" );
	int nStart = m_nStartInternalLineNoOther + m_nLinesSelOther;
	GetView()->InsertDocLines( OtherSide(), nStart,	m_astrRawLines );
	m_nStartInternalLineNoOther = nStart - m_nLinesSelOther;
	if ( m_nLineNoStart[m_nSide] == 0 )
		m_nLineNoStart[m_nSide] = 1;
	if ( m_nLineNoStart[OtherSide()] == 0 )
		m_nLineNoStart[OtherSide()] = m_nStartInternalLineNoOther;

	UpdateView( OtherSide(), (int)m_astrRawLines.GetSize() );
	SetSelFromLineNo( OtherSide(), nStart, (int)m_astrRawLines.GetSize() );

	return TRUE;
}

BOOL CUndoInsertAfter::Undo()
{
	TRACE0( "CUndoInsertAfter::Undo\n" );
	GetView()->RemoveDocLines( OtherSide(), m_nStartInternalLineNoOther + m_nLinesSelOther, 
							(int)m_astrRawLines.GetSize() );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}
