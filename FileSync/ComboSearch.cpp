// ComboSearch.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "ComboSearch.h"


// CComboSearch

IMPLEMENT_DYNAMIC(CComboSearch, CComboBox)
CComboSearch::CComboSearch()
{
}

CComboSearch::~CComboSearch()
{
}


BEGIN_MESSAGE_MAP(CComboSearch, CComboBox)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnCbnSelendok)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CComboSearch-Meldungshandler


void CComboSearch::OnCbnSelendok()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), 0), reinterpret_cast<LPARAM>(GetSafeHwnd()));
}

BOOL CComboSearch::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        int nVirtKey = (int) pMsg->wParam;
        if (nVirtKey == VK_RETURN)
		{
			CString strSearchText;
			GetWindowText( strSearchText );
			if ( strSearchText.IsEmpty() )
				return TRUE;
			CString strLB;
			if ( GetCount() > 0 )
				GetLBText( 0, strLB );
			if ( strLB.IsEmpty() || strLB.CompareNoCase( strSearchText ) != 0 )
				InsertString( 0, strSearchText );
			OnCbnSelendok();
			return TRUE;
		}
        else if (nVirtKey == VK_DELETE)
		{
			DWORD dwSel = GetEditSel();
			int nStart = LOWORD(dwSel);
			int nEnd = HIWORD(dwSel);
			if ( nStart == nEnd )
				SetEditSel( nStart, nStart+1 );
			Clear();
			return TRUE;
		}
		else if ( GetKeyState(VK_CONTROL) & 0x8000 ) {
        	if (nVirtKey == 'C')		// ^c
			{
				DWORD dwCurSel = GetEditSel();
				WORD dStart = LOWORD(dwCurSel);
				WORD dEnd   = HIWORD(dwCurSel);
				if ( dStart != dEnd )
					Copy();
				return TRUE;
			}
	        else if (nVirtKey == 'V')	// ^v
			{
				Paste();
				return TRUE;
			}
	        else if (nVirtKey == 'X')	// ^x
			{
				Cut();
				return TRUE;
			}
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

CString CComboSearch::GetSearchText()
{
	CString strSearchText;
	int n = GetCurSel();
	if ( n != LB_ERR )
		GetLBText( n, strSearchText );
	else
		GetWindowText( strSearchText );
	return strSearchText;
}
