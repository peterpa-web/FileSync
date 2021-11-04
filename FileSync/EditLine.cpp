// EditLine.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "EditLine.h"


// CEditLine

IMPLEMENT_DYNAMIC(CEditLine, CEdit)
CEditLine::CEditLine()
{
}

CEditLine::~CEditLine()
{
}


BEGIN_MESSAGE_MAP(CEditLine, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CEditLine-Meldungshandler


void CEditLine::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: ESC / Enter to exit editing
	if ( nChar == 0x1b )
	{
		DestroyWindow();		// TODO: reset ref. m_pEdit to NULL for OnEditPaste()
		return;
	}
	else if ( nChar == 0x0d || nChar == 0x0a )
	{
		GetParentOwner()->PostMessage( WM_COMMAND, IDC_EDITLINE | (BN_CLICKED << 16), NULL );
		return;
	}
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CEditLine::OnCharDelete()
{
	int nStart;
	int nEnd;
	GetSel( nStart, nEnd );
	if ( nStart == nEnd )
		SetSel( nStart, nStart+1 );
	Clear();
}

//void CEditLine::OnUpdateEditDelete(CCmdUI *pCmdUI)
//{
//	pCmdUI->Enable();
//}
