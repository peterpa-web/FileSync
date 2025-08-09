// ComboBoxDir.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "DocManFileSync.h"
#include "DocDirNative.h"
#include "DocTemplFileSync.h"
#include "ViewFileSync.h"
#include "MainFrm.h"

#include "ComboBoxDir.h"

// see also http://www.codeproject.com/combobox/combocompletion.asp

// CComboBoxDir

IMPLEMENT_DYNAMIC(CComboBoxDir, CComboBox)
CComboBoxDir::CComboBoxDir()
{
    m_bAutoComplete = TRUE;
	m_nSide = 0;
	m_pTemplate = NULL;
	m_bMouseCaptured = FALSE;
	m_bWarn = FALSE;
	m_bDirMode = FALSE;
}

CComboBoxDir::~CComboBoxDir()
{
}


BEGIN_MESSAGE_MAP(CComboBoxDir, CComboBox)
	ON_WM_DROPFILES()
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnCbnSelendok)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnCbnEditupdate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CComboBoxDir::Create(
   DWORD dwStyle,
   const RECT& rect,
   CWnd* pParentWnd,
   UINT nID 
)
{
	BOOL bRet = CComboBox::Create( dwStyle, rect, pParentWnd, nID );
	if ( bRet )
		DragAcceptFiles();
	return bRet;
}

void CComboBoxDir::SetText( LPCTSTR lpsz )
{
	SetWindowText( lpsz );
	if ( *lpsz == '\0' )
		return;

	SetWindowText( lpsz );
	int n = FindStringExact( -1, lpsz );
	TRACE2("CComboBoxDir::SetText FindStringExact=%d %s\n", n, lpsz);
	if ( n != 0 )
	{
		if ( n != CB_ERR ) {
			size_t nSize = _tcslen( lpsz );
			TCHAR szText[MAX_PATH];
			GetLBText( n, szText );
			if ( _tcsncicmp( lpsz, szText, nSize ) == 0 ) { // 20110817 instead of _tcsnccmp
				TRACE0(" DeleteString\n");
				DeleteString( n );
			} else
				return;
		}
		InsertString( 0, lpsz );
		SetCurSel( 0 );
	}
}

void CComboBoxDir::StoreList()
{
	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("ViewDir") );
	ASSERT( hSecKey != NULL );
	if (hSecKey == NULL)
		return;

	LPTSTR pszFmt = ( m_nSide == 0 ? _T("PathLeft%d") : _T("PathRight%d") );
	int nMax = GetCount();
	for ( int n = 0; n < 30; ++n )
	{
		CString strValueName;
		strValueName.Format( pszFmt, n );
		if ( n < nMax )
		{
			CString strValue;
			GetLBText( n, strValue );
			LPCTSTR pszValue = strValue;
			LONG lResult = RegSetValueEx(hSecKey, strValueName, NULL, REG_SZ,
				(LPBYTE)pszValue, (lstrlen(pszValue)+1)*sizeof(TCHAR));
		}
		else
			RegDeleteValue( hSecKey, strValueName );
	}
	RegCloseKey(hSecKey);
}

CString CComboBoxDir::RestoreList()
{
	CString strRet;
	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("ViewDir") );
	ASSERT( hSecKey != NULL );
	if (hSecKey == NULL)
		return strRet;

	LPTSTR pszFmt = ( m_nSide == 0 ? _T("PathLeft%d") : _T("PathRight%d") );
	for ( int n = 0; n < 30; ++n )
	{
		CString strValueName;
		strValueName.Format( pszFmt, n );
		TCHAR szValue[MAX_PATH];
		DWORD dwSize = MAX_PATH * sizeof(TCHAR);
		LONG lResult = RegQueryValueEx(hSecKey, strValueName, NULL, NULL,
			(LPBYTE)szValue, &dwSize );
		if ( lResult == ERROR_SUCCESS )
		{
			InsertString( -1, szValue );
			if ( n == 0 )
				strRet = szValue;
		}
	}
	RegCloseKey(hSecKey);
	return strRet;
}

// CComboBoxDir-Meldungshandler


void CComboBoxDir::OnDropFiles(HDROP hDropInfo)
{
	POINT pt;
	TCHAR szFile[MAX_PATH+1];
	if ( DragQueryPoint( hDropInfo, &pt ) )
	{
	    AdjustTitleTip(-1);

		UINT nRet = DragQueryFile( hDropInfo, 0, szFile, MAX_PATH );
		ASSERT( nRet != 0 );
		TCHAR szPath[MAX_PATH+1];
		CDocManFileSync::ResolveShortcut(szPath, szFile);
		DoOpen( szPath, FALSE );
	}
	DragFinish( hDropInfo );

	CComboBox::OnDropFiles(hDropInfo);
}

void CComboBoxDir::OnCbnSelendok()
{
	CString strFile;
	int n = GetCurSel();
	if ( n != LB_ERR )
		GetLBText( n, strFile );
	else
		GetWindowText( strFile );
	if ( strFile.Left(2) == _T("\\\\") && ! CDocDir::CheckConn(strFile, TRUE) ) {
		//Beep(100, 100);
		MessageBeep(MB_ICONWARNING);
		m_bWarn = TRUE;
		SetCurSel(-1);
		return;
	}
	if ( strFile.IsEmpty() )
		return;

	DoOpen( strFile, m_bDirMode );
//	m_bAutoComplete = TRUE;
}

BOOL bOpenBusy = FALSE;

void CComboBoxDir::DoOpen( CString strFile, BOOL bDirMode )
{
	TRACE0("CComboBoxDir::DoOpen ====================== \n");
	if ( bOpenBusy ) {
		TRACE0("CComboBoxDir::DoOpen busy \n");
		return;
	}
	bOpenBusy = TRUE;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	int result = _taccess( strFile, 0 );	// 0 == found
	if ( result != 0 || bDirMode )
	{
		((CViewFileSync*)GetParent())->ChangedSelCombo( m_nSide, TRUE );
		CDocument *pDoc = pDM->OpenDocumentDir( strFile );
		if ( pDoc != NULL && ((CDocDir*)pDoc)->CheckPath() ) {
			m_bWarn = FALSE;
			bOpenBusy = FALSE;
			return;
		}
	}
	else {
		if ( result == 0 )
		{
			BOOL bDir = IsDir( strFile );
			BOOL bReset = bDir;
			if ( !bDir && m_pTemplate == NULL ) {
				CDocTemplFileSync *pTempl = (CDocTemplFileSync *)(pDM->GetBestTemplate( strFile ));
				// bReset |= (pTempl->GetIconNo() == 3);	// ZIP
				bReset |= pTempl->IsDocKindOf(RUNTIME_CLASS(CDocDir));
			}
			((CViewFileSync*)GetParent())->ChangedSelCombo( m_nSide, bReset );
			CDocument *pDoc;
			if ( bDir )
				pDoc = pDM->OpenDocumentDir( strFile );
			else
				pDoc = pDM->OpenDocumentFile( strFile, m_pTemplate );
			m_bWarn = (pDoc == NULL);
			bOpenBusy = FALSE;
			return;
		}
	}
	//Beep(100, 100);
	MessageBeep(MB_ICONWARNING);
	m_bWarn = TRUE;
	bOpenBusy = FALSE;
}

BOOL CComboBoxDir::PreTranslateMessage(MSG* pMsg)
{
    // Need to check for backspace/delete. These will modify the text in
    // the edit box, causing the auto complete to just add back the text
    // the user has just tried to delete. 

    if (pMsg->message == WM_KEYDOWN)
    {
        m_bAutoComplete = TRUE;

        int nVirtKey = (int) pMsg->wParam;
        if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK)
            m_bAutoComplete = FALSE;
        if (nVirtKey == VK_RETURN)
		{
			CString str;
			GetWindowText( str );
//			int result = _taccess( str, 0 ); // del 20100219
//			if ( result == 0 )
//			{
				SetWindowText( str );
				OnCbnSelendok();
				return TRUE;
//			}
		}
        else if (nVirtKey == VK_DELETE)
		{
			DWORD dwSel = GetEditSel();
			int nStart = LOWORD(dwSel);
			int nEnd = HIWORD(dwSel);
			if ( nStart == nEnd )
				SetEditSel( nStart, nStart+1 );
			Clear();
		}
    }
	else if (pMsg->message == WM_LBUTTONDOWN)
	{
		((CViewFileSync*)GetParent())->SelectSide2( (CViewFileSync::Side)m_nSide );
		if (m_bMouseCaptured && pMsg->message == WM_LBUTTONDOWN)
			AdjustTitleTip(-1);
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

void CComboBoxDir::OnCbnEditupdate()
{
	// if we are not to auto update the text, get outta here
	if (!m_bAutoComplete) 
		return;

	// Get the text in the edit box
	CString str;
	GetWindowText(str);
	int nLength = str.GetLength();

	// Currently selected range
	DWORD dwCurSel = GetEditSel();
	WORD dStart = LOWORD(dwCurSel);
	WORD dEnd   = HIWORD(dwCurSel);

	// Search for, and select in, and string in the combo box that is prefixed
	// by the text in the edit box
	if (SelectString(-1, str) == CB_ERR || dEnd < str.GetLength())
	{
		SetWindowText(str);            // No text selected, so restore what 
										// was there before
		if (dwCurSel != CB_ERR)
			SetEditSel(dStart, dEnd);    //restore cursor postion
	}

	// Set the text selection as the additional text that we have added
	if (dEnd < nLength && dwCurSel != CB_ERR)
		SetEditSel(dStart, dEnd);
	else
		SetEditSel(nLength, -1);
}

BOOL CComboBoxDir::IsDir( const CString &strPath ) const
{
	int nBS = 0;	// backslash counter
	if ( strPath.Left(2) == _T("\\\\") )		// check share names only
	{
		for ( int n=0; n < strPath.GetLength(); ++n )
		{
			if ( strPath[n] == '\\' )
				++nBS;
		}
	}
	if ( nBS != 3 )	// check for dirs excluding path names like \\server\c$
	{
		struct _stati64 fs;
		int result = _tstati64( strPath, &fs );
		if ( result != 0 || (fs.st_mode & _S_IFDIR) == 0 )
			return FALSE;
	}
	return TRUE;
}

void CComboBoxDir::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	DWORD dwCurSel = GetEditSel();
	WORD dStart = LOWORD(dwCurSel);
	WORD dEnd   = HIWORD(dwCurSel);
	pCmdUI->Enable( dStart != dEnd );
}

void CComboBoxDir::OnEditCopy()
{
	Copy();
}

void CComboBoxDir::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}

void CComboBoxDir::OnEditPaste()
{
	Paste();
}

void CComboBoxDir::OnUpdateEditCut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
}

void CComboBoxDir::OnEditCut()
{
	Cut();
}

void CComboBoxDir::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( FALSE );
}

void CComboBoxDir::OnEditDelete()
{
	Clear();
}

HWND CComboBoxDir::GetEditWnd()
{
	COMBOBOXINFO cbi;
	cbi.cbSize = sizeof(cbi);
	if ( GetComboBoxInfo( &cbi ) )
		return cbi.hwndItem;

	return NULL;
}
int CComboBoxDir::GetIdealItemRect(int nIndex, LPRECT lpRect)
{
    // Calculate the ideal rect for an item. The ideal rect is dependent
    // on the length of the string. This only works for regular 
    // (non owner-draw)listboxes.
    ASSERT(lpRect != NULL);
    ASSERT(nIndex >= 0);
    DWORD dwStyle = GetStyle();
//    int    nStatus = GetItemRect(nIndex, lpRect);
//	GetClientRect(lpRect);
	::GetClientRect(GetEditWnd(), lpRect);
	lpRect->right -= 3;
//    if (nStatus != LB_ERR && !(dwStyle & LBS_OWNERDRAWFIXED) && 
//        !(dwStyle & LBS_OWNERDRAWVARIABLE))
    {
        CString strItem;
//        GetText(nIndex, strItem);
		GetWindowText(strItem);
        if (!strItem.IsEmpty())
        {
            // Calulate the ideal text length.
            CClientDC DC(this);
            CFont* pOldFont = DC.SelectObject(GetFont());
            CSize ItemSize = DC.GetTextExtent(strItem);
            DC.SelectObject(pOldFont);

            // Take the maximum of regular width and ideal width.
            const int cxEdgeSpace = 2;
            lpRect->right = max(lpRect->right, 
                lpRect->left + ItemSize.cx + (cxEdgeSpace * 2));
        }
    }
//    else
//    {
//        TRACE("Owner-draw listbox detected - override CTitleTipListBox::GetIdeaItemRect()\n");
//    }
//    return nStatus;
	return 1;
}

void CComboBoxDir::AdjustTitleTip(int nNewIndex)
{
    if (!::IsWindow(m_TitleTip.m_hWnd))
    {
        VERIFY(m_TitleTip.Create(this));
    }

    if (nNewIndex == -1)
    {
        m_TitleTip.Hide();
    }
    else
    {
        CRect idealItemRect;
        GetIdealItemRect(nNewIndex, idealItemRect);
        CRect itemRect;
//        GetItemRect(nNewIndex, ItemRect);
//		GetClientRect( ItemRect );
		::GetClientRect(GetEditWnd(), itemRect);
		itemRect.right -= 3;
        if (itemRect == idealItemRect)
        {
            m_TitleTip.Hide();
        }
        else
        {
            // Adjust the rect for being near the edge of screen.
            ClientToScreen(idealItemRect);
//            int nScreenWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);

			HMONITOR hMon = MonitorFromRect( idealItemRect, MONITOR_DEFAULTTONEAREST );
			MONITORINFO mInfo;
			mInfo.cbSize = sizeof(MONITORINFO);
			if (GetMonitorInfo(hMon, &mInfo)) {
				CRect rWork(mInfo.rcWork);
	            if (idealItemRect.right > rWork.right)
	            {
	                idealItemRect.OffsetRect(rWork.right - idealItemRect.right, 0);
	            }
	            if (idealItemRect.left < rWork.left)
	            {
	                idealItemRect.OffsetRect(rWork.left - idealItemRect.left, 0);
	            }
			}
//            m_TitleTip.Show(IdealItemRect, nNewIndex);  
			idealItemRect.OffsetRect(0, -14);	// on top of ctrl
            m_TitleTip.Show(idealItemRect);  
        }
    }

    if (m_TitleTip.IsWindowVisible())
    {
        // Make sure we capture mouse so we can detect when to turn off 
        // title tip.
        if (!m_bMouseCaptured && GetCapture() != this)
        {
            CaptureMouse();
        }
    }
    else
    {
        // The tip is invisible so release the mouse.
        if (m_bMouseCaptured)
        {
            VERIFY(ReleaseCapture());
            m_bMouseCaptured = FALSE;
        }
    }
}

void CComboBoxDir::CaptureMouse()
{
    ASSERT(!m_bMouseCaptured);
    CPoint point;
    VERIFY(GetCursorPos(&point));
    ScreenToClient(&point);
    m_LastMouseMovePoint = point;
    SetCapture();
    m_bMouseCaptured = TRUE;
}


HBRUSH CComboBoxDir::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_LISTBOX) 
	{ 
		if (m_listbox.GetSafeHwnd() == NULL) 
		{ 
			TRACE(_T("subclassing listbox\n")); 
			m_listbox.SubclassWindow(pWnd->GetSafeHwnd()); 
		} 
	} 
	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor); 
	if (m_bWarn && nCtlColor == CTLCOLOR_EDIT) 
	{
		COLORREF cWarn = RGB(255,255,0);
		if ( ((HBRUSH)m_brushBk) == NULL )
			m_brushBk.CreateSolidBrush( cWarn );
		pDC->SetBkColor( cWarn );
		hbr = m_brushBk;
	}
	return hbr;
}


void CComboBoxDir::OnDestroy()
{
    AdjustTitleTip(-1);
    m_TitleTip.DestroyWindow();

	if (m_listbox.GetSafeHwnd() != NULL) 
		m_listbox.UnsubclassWindow(); 

	CComboBox::OnDestroy();
}


void CComboBoxDir::OnMouseMove(UINT nFlags, CPoint point)
{
    if (point != m_LastMouseMovePoint)
    {
        m_LastMouseMovePoint = point;

//        int nIndexHit = m_nNoIndex;
		int nIndexHit = -1;

        CRect clientRect;
		GetClientRect(clientRect);
        if (IsAppActive() && clientRect.PtInRect(point))
        {
            // Hit test.
//            for (int n = 0; nIndexHit == m_nNoIndex && n < GetCount(); n++)
//            {
//                CRect ItemRect;
//                GetItemRect(n, ItemRect);
//                if (ItemRect.PtInRect(point))
//                {
//                    nIndexHit = n;    
//                }
//            }
	        CRect editRect;
			::GetClientRect(GetEditWnd(), editRect);
			if (point.x <= (editRect.right + 3))
				nIndexHit = 0;
        }
        AdjustTitleTip(nIndexHit);
    }

	CComboBox::OnMouseMove(nFlags, point);
}


void CComboBoxDir::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);

    if (pNewWnd != &m_TitleTip)
    {
        AdjustTitleTip(-1);
		if ( m_listbox.m_hWnd != NULL )
			m_listbox.PostMessage(WM_KILLFOCUS);
    }
}

void CComboBoxDir::Highlight(BOOL b)
{
	if ( b )
		::PostMessage(m_hWnd, CB_SETEDITSEL, 0, MAKELONG(0, -1));
//		SetEditSel( 0, -1 );
	else
		::PostMessage(m_hWnd, CB_SETEDITSEL, 0, MAKELONG(-1, -1));
//		SetEditSel( -1, -1 );
}
/*
void CComboBoxDir::ClearListTop()
{
	SetWindowText(L"");
	int n = GetCurSel();
	if (n >= 0)
		DeleteString(n);
}
*/
BOOL CComboBoxDir::IsAppActive() 
{ 
	return ((CMainFrame*)GetParentFrame())->IsAppActive(); 
}
