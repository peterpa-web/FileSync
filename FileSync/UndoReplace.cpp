#include "StdAfx.h"
#include "UndoReplace.h"

CUndoReplace::CUndoReplace(CViewText *pView) :
		CUndoViewText(pView)
{
}

CUndoReplace::~CUndoReplace(void)
{
}

BOOL CUndoReplace::DoFirst(void)
{
	TRACE0( "CUndoReplace::DoFirst\n" );
	CUndoViewText::DoFirst();
	GetView()->CopyDocLines( OtherSide(), m_nSelAnchor, m_nSelCount, m_astrDelLines );
	return TRUE;
}

BOOL CUndoReplace::Do()
{
	TRACE0( "CUndoReplace::Do\n" );
	GetView()->RemoveDocLines( OtherSide(), m_nStartInternalLineNoOther, (int)m_astrDelLines.GetSize() );
	GetView()->InsertDocLines( OtherSide(), m_nStartInternalLineNoOther, m_astrRawLines );
	if ( m_nLineNoStart[m_nSide] == 0 )
		m_nLineNoStart[m_nSide] = 1;
	if ( m_nLineNoStart[OtherSide()] == 0 )
		m_nLineNoStart[OtherSide()] = m_nStartInternalLineNoOther;

	UpdateView( OtherSide(), (int)m_astrRawLines.GetSize() - (int)m_astrDelLines.GetSize() );
	SetSelFromLineNo( OtherSide(), m_nStartInternalLineNoOther, (int)m_astrRawLines.GetSize() );

	return TRUE;
}

BOOL CUndoReplace::Undo()
{
	TRACE0( "CUndoReplace::Undo\n" );
	GetView()->RemoveDocLines( OtherSide(), m_nStartInternalLineNoOther, (int)m_astrRawLines.GetSize() );
	GetView()->InsertDocLines( OtherSide(), m_nStartInternalLineNoOther, m_astrDelLines );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}

INT_PTR CUndoReplace::GetSize()
{
	return CUndoViewText::GetSize() + m_astrDelLines.GetSize() * 80;
}
