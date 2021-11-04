#include "StdAfx.h"

#include "UndoInsertText.h"

CUndoInsertText::CUndoInsertText(CViewText *pView, HGLOBAL hg) :
		CUndoViewText(pView)
{
	m_hg = hg;
}

CUndoInsertText::~CUndoInsertText(void)
{
}

BOOL CUndoInsertText::DoFirst(void)
{
	if ( !PrepareRange() )
		return FALSE;
	m_nSelCount = 0;	// don't delete selected items

	if ( !SaveLines() )
		return FALSE;

	if ( m_hg == NULL )
		return FALSE;

	LPTSTR lptstr = (LPTSTR)GlobalLock( m_hg ); 
	if (lptstr != NULL) 
	{
		LPTSTR pc0 = lptstr;
		LPTSTR pc;
		for ( pc = lptstr; *pc != (TCHAR)0; ++pc )
		{
			if ( *pc == (TCHAR)'\r' && pc[1] == (TCHAR)'\n' )
			{
				int nLen = (int)(pc - pc0);
				m_astrInsLines.Add( CString( pc0, nLen ) );
				++pc;
				pc0 = pc + 1;
			}
		}
		if ( pc0 < pc )
		{
			int nLen = (int)(pc - pc0);
			m_astrInsLines.Add( CString( pc0, nLen ) );
		}
		GlobalUnlock( m_hg ); 
	} 

	GetView()->CopyDocLines( m_nSide, m_nSelAnchor, m_nSelCount, m_astrRawLines );

	return TRUE;
}

BOOL CUndoInsertText::Do()
{
	TRACE0( "CUndoInsertText::Do\n" );

	// delete selected items, if any ...
	GetView()->RemoveDocLines( m_nSide, m_nStartInternalLineNo, (int)m_astrRawLines.GetSize() );

	// copy to this ...
	GetView()->InsertDocLines( m_nSide, m_nLineNoSel, m_astrInsLines );

	UpdateView( m_nSide, (int)m_astrInsLines.GetSize() - (int)m_astrRawLines.GetSize() );

	SetSelFromLineNo( m_nSide, m_nStartInternalLineNo, (int)m_astrInsLines.GetSize() );
	return TRUE;
}

BOOL CUndoInsertText::Undo()
{
	TRACE0( "CUndoInsertText::Undo\n" );
	GetView()->RemoveDocLines( m_nSide, m_nStartInternalLineNo, (int)m_astrInsLines.GetSize() );
	GetView()->InsertDocLines( m_nSide, m_nStartInternalLineNo, m_astrRawLines );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}

INT_PTR CUndoInsertText::GetSize()
{
	return CUndoViewText::GetSize() + m_astrInsLines.GetSize() * 80;
}
