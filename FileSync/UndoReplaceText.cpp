#include "StdAfx.h"
#include "DocText.h"

#include "UndoReplaceText.h"

CUndoReplaceText::CUndoReplaceText(CViewText *pView, HGLOBAL hg) :
		CUndoViewText(pView)
{
	m_hg = hg;
}

CUndoReplaceText::~CUndoReplaceText(void)
{
}

BOOL CUndoReplaceText::DoFirst(void)
{
	if ( !PrepareRange() )
		return FALSE;
//	m_nSelCount = 0;	// don't delete selected items

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

	if ( m_nLineNoStart[m_nSide] == 0 && (GetDoc(m_nSide)->GetLineCount() > m_nLinesSel ) )	{	
		TRACE0( "CUndoReplaceText::DoFirst adj LineNoStart own\n" );
		m_nLineNoStart[m_nSide] = 1;
	}
	if ( m_nLineNoStart[OtherSide()] == 0 && (GetDoc(OtherSide())->GetLineCount() > 0 )) {
		TRACE0( "CUndoReplaceText::DoFirst adj LineNoStart other\n" );
		m_nLineNoStart[OtherSide()] = 1;
	}
	return TRUE;
}

BOOL CUndoReplaceText::Do()
{
	TRACE0( "CUndoReplaceText::Do\n" );

	// delete selected items, if any ...
	GetView()->RemoveDocLines( m_nSide, m_nStartInternalLineNo, (int)m_astrRawLines.GetSize() );

	// copy to this ...
	GetView()->InsertDocLines( m_nSide, m_nLineNoSel, m_astrInsLines );

	UpdateView( m_nSide, (int)m_astrInsLines.GetSize() - (int)m_astrRawLines.GetSize() );

	SetSelFromLineNo( m_nSide, m_nStartInternalLineNo, (int)m_astrInsLines.GetSize() );
	return TRUE;
}

BOOL CUndoReplaceText::Undo()
{
	TRACE0( "CUndoReplaceText::Undo\n" );
	GetView()->RemoveDocLines( m_nSide, m_nStartInternalLineNo, (int)m_astrInsLines.GetSize() );
	GetView()->InsertDocLines( m_nSide, m_nStartInternalLineNo, m_astrRawLines );
	RestoreItemData();
	SetSelection( m_nSide, m_nSelAnchor, m_nSelCount );
	return TRUE;
}

INT_PTR CUndoReplaceText::GetSize()
{
	return CUndoViewText::GetSize() + m_astrInsLines.GetSize() * 80;
}
