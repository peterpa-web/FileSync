#include "StdAfx.h"
#include "UndoInsertBefore.h"

CUndoInsertBefore::CUndoInsertBefore(CViewText *pView) :
		CUndoViewText(pView)
{
}

CUndoInsertBefore::~CUndoInsertBefore(void)
{
}

BOOL CUndoInsertBefore::Do()
{
	TRACE0( "CUndoInsertBefore::Do\n" );
	GetView()->InsertDocLines( OtherSide(), m_nStartInternalLineNoOther, m_astrRawLines );
	if ( m_nLineNoStart[m_nSide] == 0 )
		m_nLineNoStart[m_nSide] = 1;
	if ( m_nLineNoStart[OtherSide()] == 0 )
		m_nLineNoStart[OtherSide()] = m_nStartInternalLineNoOther;

	UpdateView( OtherSide(), (int)m_astrRawLines.GetSize() );
	SetSelFromLineNo( OtherSide(), m_nStartInternalLineNoOther, (int)m_astrRawLines.GetSize() );

	return TRUE;
}

BOOL CUndoInsertBefore::Undo()
{
	TRACE0( "CUndoInsertBefore::Undo\n" );
	GetView()->RemoveDocLines( OtherSide(), m_nStartInternalLineNoOther, (int)m_astrRawLines.GetSize() );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}
