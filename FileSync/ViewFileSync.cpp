// ViewFileSync.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "DocFileSync.h"
#include "DocManFileSync.h"
#include "DocTemplFileSync.h"
#include "AssocDlg.h"
#include "ColorsDlg.h"
#include "afxpriv.h"		// AfxLoadString
#include "MainFrm.h"

#include "ViewFileSync.h"

DWORD CALLBACK CopyProgressRoutine(
	__in      LARGE_INTEGER TotalFileSize,
	__in      LARGE_INTEGER TotalBytesTransferred,
	__in      LARGE_INTEGER StreamSize,
	__in      LARGE_INTEGER StreamBytesTransferred,
	__in      DWORD dwStreamNumber,
	__in      DWORD dwCallbackReason,
	__in      HANDLE hSourceFile,
	__in      HANDLE hDestinationFile,
	__in_opt  LPVOID lpData
)
{
	REBProgressManager *progressMan = (REBProgressManager *)lpData;
	// Sleep(200); // test only!
	progressMan->AddProgressBased( TotalBytesTransferred.QuadPart );
	if ( progressMan->GetUserAbortFlag() )
		return PROGRESS_CANCEL;

	return PROGRESS_CONTINUE;
}

// CViewFileSync
CViewFileSync::Side CViewFileSync::s_nSide = CViewFileSync::left;
BOOL CViewFileSync::m_bCompareEnabled = TRUE;

COLORREF CViewFileSync::m_crBkColor = ::GetSysColor(COLOR_BTNFACE);
COLORREF CViewFileSync::m_crWndColor = ::GetSysColor(COLOR_WINDOW);

IMPLEMENT_DYNAMIC(CViewFileSync, CView)

CViewFileSync::CViewFileSync()
{
	TRACE0( "CViewFileSync::CViewFileSync()\n" );
//	m_bUseTree = FALSE;
	m_pDoc[0] = NULL;
	m_pDoc[1] = NULL;
	m_nTitleID = 0;
	m_nMenueID = 0;
	m_nToolbarID[0] = 0;
	m_nToolbarID[1] = 0;
	m_bSaveMore = TRUE;		// has Save and More buttons
	m_bChanged[0] = FALSE;
	m_bChanged[1] = FALSE;
	m_pListMode = NULL;
	m_pProgressMan = NULL; // see PreCreateWindow
}

CViewFileSync::~CViewFileSync()
{
	RemoveListMode();
}

BEGIN_MESSAGE_MAP(CViewFileSync, CView)
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_OPENDIRRIGHT, OnFileOpenDirRight)
	ON_COMMAND(ID_FILE_OPENDIRLEFT, OnFileOpenDirLeft)
	ON_COMMAND(ID_FILE_OPEN_RIGHT, OnFileOpenRight)
	ON_COMMAND(ID_FILE_OPEN_LEFT, OnFileOpenLeft)
//	ON_COMMAND(IDC_BUTTON_OPEN_RIGHT, OnFileOpenRight)
//	ON_COMMAND(IDC_BUTTON_OPEN_LEFT, OnFileOpenLeft)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_LEFT, OnUpdateFileSaveLeft)
	ON_COMMAND(ID_FILE_SAVE_LEFT, OnFileSaveLeft)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_RIGHT, OnUpdateFileSaveRight)
	ON_COMMAND(ID_FILE_SAVE_RIGHT, OnFileSaveRight)
//	ON_COMMAND(ID_FILE_MORE_LEFT, OnFileMoreLeft)
//	ON_COMMAND(ID_FILE_MORE_RIGHT, OnFileMoreRight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DIRCOMP, OnUpdateViewDircomp)
	ON_COMMAND(ID_VIEW_DIRCOMP, OnViewDircomp)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_LEFT_AS, OnFileSaveLeftAs)
	ON_COMMAND(ID_FILE_SAVE_RIGHT_AS, OnFileSaveRightAs)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_VIEW_ASSOCIATIONS, OnViewAssociations)
	ON_COMMAND(ID_VIEW_COLORS, OnViewColors)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
	ON_COMMAND(ID_FILE_MODELEFT, OnFileModeleft)
	ON_COMMAND(ID_FILE_MODERIGHT, OnFileModeright)
	ON_COMMAND(ID_VIEW_CHECKLEFT, OnViewCheckleft)
	ON_COMMAND(ID_VIEW_CHECKRIGHT, OnViewCheckright)
	ON_UPDATE_COMMAND_UI(ID_FILE_MODELEFT, OnUpdateFileModeleft)
	ON_UPDATE_COMMAND_UI(ID_FILE_MODERIGHT, OnUpdateFileModeright)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHECKLEFT, &CViewFileSync::OnUpdateViewCheckleft)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHECKRIGHT, &CViewFileSync::OnUpdateViewCheckright)
	ON_COMMAND(ID_VIEW_TEXT, &CViewFileSync::OnViewText)
	ON_COMMAND(ID_HELP_CONTENTS, &CViewFileSync::OnHelpContents)
	ON_COMMAND(ID_VIEW_XML, &CViewFileSync::OnViewXml)
	ON_COMMAND(ID_VIEW_HEX, &CViewFileSync::OnViewHex)
END_MESSAGE_MAP()


BOOL CViewFileSync::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= /* WS_VSCROLL | */ WS_HSCROLL;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1), NULL);

	return TRUE;
}

int CViewFileSync::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	TRACE0( "CViewFileSync::OnCreate()\n" );

	m_imagelist.Create( IDB_TREEIMAGES, 16, 6, RGB( 0, 128, 128 ) );

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// if ok, wire in the current document
	ASSERT(m_pDocument == NULL);
	CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;

	// A view should be created in a given context!
	if (pContext != NULL )
	{
		if ( pContext->m_pCurrentDoc == NULL )
			pContext->m_pCurrentDoc = (CDocFileSync*)pContext->m_pNewDocTemplate->CreateNewDocument();

		if ( m_pDoc[0] == NULL )
			m_pDoc[0] = (CDocFileSync*)pContext->m_pCurrentDoc;
		if ( m_pDoc[1] == NULL )
			m_pDoc[1] = (CDocFileSync*)pContext->m_pNewDocTemplate->CreateNewDocument();
		ASSERT(m_pDoc[0] != NULL);
		ASSERT(m_pDoc[1] != NULL);
	}
	else
	{
		TRACE0("Warning: Creating a pane with no CDocument.\n");
	}
	m_pProgressMan = GetParentFrame()->GetProgressMan();
	return 0;
}

void CViewFileSync::ChangedSelCombo( int nSide, BOOL bReset )
{
	SelectSide( nSide );
}

void CViewFileSync::SelectSide( Side s )
{
	ASSERT(s != common); 
	s_nSide = s;
	if ( s == left ) {
		m_comboDirLeft.Highlight(TRUE);
		m_comboDirRight.Highlight(FALSE);
	}
	else {
		m_comboDirLeft.Highlight(FALSE);
		m_comboDirRight.Highlight(TRUE);
	}
}

void CViewFileSync::SelectSide2( Side s )
{
	ASSERT(s != common); 
	s_nSide = s;
	if ( s == left ) {
		m_comboDirRight.Highlight(FALSE);
	}
	else {
		m_comboDirLeft.Highlight(FALSE);
	}
}

void CViewFileSync::SetDocument( CDocFileSync *pDocument, int nSide )
{
	TRACE0( "CViewFileSync::SetDocument()\n" );
	m_pDoc[ nSide ] = pDocument;
	m_strOldPath[nSide] = pDocument->GetPathNameView();
//	CToolButton* pButton = nSide == left ? &m_buttonsLeft[1] : &m_buttonsRight[1];
	if ( nSide == left )
	{
		m_comboDirLeft.SetText( m_strOldPath[nSide] );
		m_comboDirLeft.Warn( FALSE );
//		pButton->SetDlgCtrlID(ID_FILE_MODELEFT);
	}
	else
	{
		m_comboDirRight.SetText( m_strOldPath[nSide] );
		m_comboDirRight.Warn( FALSE );
//		pButton->SetDlgCtrlID(ID_FILE_MODERIGHT);
	}
	m_undoBuffer.RemoveAll();

//	pButton->LoadBitmapsEx( GetModeBitmap(nSide) );

	if ( !m_bCompareEnabled )
		return;

	BOOL b = CompareView();
	ASSERT( b );

	UpdateEditDir( 1 - nSide );
	SelectSide( s_nSide );	// highlight current and don't change s_nSide
}

void CViewFileSync::UpdateEditDir( int nSide, BOOL bStore /* = TRUE */ )
{
	TRACE0( "CViewFileSync::UpdateEditDir()\n" );
	m_strOldPath[nSide] = m_pDoc[nSide]->GetPathNameView();
	CComboBoxDir *pEdit = (nSide == left ? &m_comboDirLeft : &m_comboDirRight);
	if ( bStore ) {
		pEdit->SetText( m_strOldPath[nSide] );
	} else {
		pEdit->DeleteString( 0 );
		pEdit->SetWindowText( m_strOldPath[nSide] );
	}
}

void CViewFileSync::SaveHistory()
{
	m_comboDirLeft.StoreList();
	m_comboDirRight.StoreList();
}

void CViewFileSync::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


void CViewFileSync::OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo)
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class for treectrl
}


// CViewFileSync diagnostics

#ifdef _DEBUG
void CViewFileSync::AssertValid() const
{
	CView::AssertValid();
}

void CViewFileSync::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CViewFileSync message handlers

void CViewFileSync::OnInitialUpdate()
{
	if ( m_fontList.m_hObject != NULL )		// reinit view
	{
		TRACE1( "CViewFileSync::OnInitialUpdate() reinit %s\n", CString(GetRuntimeClass()->m_lpszClassName) );
		OnUpdate(NULL, 0, NULL);        // initial update - see CView::OnInitialUpdate
		return;
	}

	TRACE1( "CViewFileSync::OnInitialUpdate() %s\n", CString(GetRuntimeClass()->m_lpszClassName) );
	m_menu.LoadMenu( m_nMenueID );
	ASSERT( m_menu != NULL );

	m_bmFileOpen.LoadBitmap(IDB_OPEN_MENU);
	m_bmDirOpen.LoadBitmap(IDB_OPENDIR_MENU);
	m_bmDirComp.LoadBitmap(IDB_DIRCOMP);
	m_bmFileComp.LoadBitmap(IDB_FILECOMP);

//	int nX = GetSystemMetrics(SM_CXMENUCHECK); = 13
//	int nY = GetSystemMetrics(SM_CYMENUCHECK);
	m_menu.SetMenuItemBitmaps(ID_FILE_OPEN_LEFT, MF_BYCOMMAND, &m_bmFileOpen, NULL);
	m_menu.SetMenuItemBitmaps(ID_FILE_OPEN_RIGHT, MF_BYCOMMAND, &m_bmFileOpen, NULL);
	m_menu.SetMenuItemBitmaps(ID_FILE_OPENDIRLEFT, MF_BYCOMMAND, &m_bmDirOpen, NULL);
	m_menu.SetMenuItemBitmaps(ID_FILE_OPENDIRRIGHT, MF_BYCOMMAND, &m_bmDirOpen, NULL);
	m_menu.SetMenuItemBitmaps(ID_VIEW_DIRCOMP, MF_BYCOMMAND, &m_bmDirComp, NULL);
	m_menu.SetMenuItemBitmaps(ID_VIEW_FILESYNC, MF_BYCOMMAND, &m_bmFileComp, NULL);

	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	// 10 point height Courier New font
	CDC* pDC = GetDC();
	int	cyPerInch = pDC->GetDeviceCaps(LOGPIXELSY);
	ReleaseDC( pDC );
	logfont.lfHeight = -MulDiv(10, cyPerInch, 72);
	logfont.lfWeight = FW_NORMAL;
	logfont.lfPitchAndFamily = FIXED_PITCH;
	static TCHAR BASED_CODE szFaceName[] = _T("Courier New");
	lstrcpy(logfont.lfFaceName, szFaceName);
	m_fontList.CreateFontIndirect( &logfont );

	logfont.lfHeight = -MulDiv(8, cyPerInch, 72);
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	static TCHAR BASED_CODE szFaceName2[] = _T("Arial");
	lstrcpy(logfont.lfFaceName, szFaceName2);
	m_fontEdit.CreateFontIndirect( &logfont );
	
//	CRect rectB(0,0,20,20);
//	if ( m_bSaveMore )
//	{
//		VERIFY( m_buttonSaveLeft.Create( _T("Save left"), WS_VISIBLE | WS_DISABLED | 
//						BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW , rectB, this, ID_FILE_SAVE_LEFT ) );
//		m_buttonSaveLeft.LoadBitmaps( IDB_SAVE, IDB_SAVE_ACT, NULL, IDB_SAVE_DIS );
//		VERIFY( m_buttonMoreLeft.Create( _T("More left"), WS_VISIBLE |  
//						BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW , rectB, this, ID_FILE_MORE_LEFT ) );
//		m_buttonMoreLeft.LoadBitmaps( IDB_MORE, IDB_MORE_ACT, NULL, NULL );
//		VERIFY( m_buttonSaveRight.Create( _T("Save right"), WS_VISIBLE | WS_DISABLED | 
//						BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW , rectB, this, ID_FILE_SAVE_RIGHT ) );
//		m_buttonSaveRight.LoadBitmaps( IDB_SAVE, IDB_SAVE_ACT, NULL, IDB_SAVE_DIS );
//		VERIFY( m_buttonMoreRight.Create( _T("More right"), WS_VISIBLE |  
//						BS_BITMAP | BS_PUSHBUTTON | BS_OWNERDRAW , rectB, this, ID_FILE_MORE_RIGHT ) );
  //		m_buttonMoreRight.LoadBitmaps( IDB_MORE, IDB_MORE_ACT, NULL, NULL );
//		m_buttonMoreRight.LoadBitmaps( IDB_MORE, NULL, NULL, NULL );
//	}

	CRect rectE(80,5,200,15);
//	VERIFY( m_comboDirLeft.Create(WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, rectE, this, IDC_EDIT_DIR_LEFT ) );
	VERIFY( m_comboDirLeft.Create(WS_VISIBLE | WS_VSCROLL | ES_READONLY | CBS_AUTOHSCROLL | CBS_DROPDOWN, 
			rectE, this, IDC_EDIT_DIR_LEFT ) );
	m_comboDirLeft.SetFont( &m_fontEdit );
	m_comboDirLeft.SetSide( 0 );
	rectE.right = rectE.Width() + 250;
	rectE.left = 250;
//	VERIFY( m_comboDirRight.Create(WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, rectE, this, IDC_EDIT_DIR_LEFT ) );
	VERIFY( m_comboDirRight.Create(WS_VISIBLE | WS_VSCROLL | ES_READONLY | CBS_AUTOHSCROLL | CBS_DROPDOWN, 
			rectE, this, IDC_EDIT_DIR_RIGHT ) );
	m_comboDirRight.SetFont( &m_fontEdit );
	m_comboDirRight.SetSide( 1 );

	EnableToolTips();

	OnInitialUpdate2();
	CView::OnInitialUpdate();
}

void CViewFileSync::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if ( m_buttonsLeft[0].m_hWnd == NULL )
		return;		// not initialized

	// buttons
	int nCxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	int cx2 = (cx - nCxVScroll) / 2;
	CRect rect;
	rect.top = 1;
	rect.bottom = rect.top + 19;
	rect.left = 1;
	rect.right = rect.left + 19;
	m_buttonsLeft[0].MoveWindow( rect );
	m_buttonsLeft[0].SizeToContent();
	if ( m_buttonsLeft[1].m_hWnd != NULL )
	{
		rect.right = rect.Width() + 21;
		rect.left = 21;
		m_buttonsLeft[1].MoveWindow( rect );
		m_buttonsLeft[1].SizeToContent();
	}
	if ( m_buttonsLeft[2].m_hWnd != NULL )
	{
		rect.right = rect.Width() + 21 + 20;
		rect.left = 21 + 20;
		m_buttonsLeft[2].MoveWindow( rect );
		m_buttonsLeft[2].SizeToContent();
	}
	rect.right = rect.Width() + cx2 + 3;
	rect.left = cx2 + 3;
	m_buttonsRight[0].MoveWindow( rect );
	m_buttonsRight[0].SizeToContent();
	if ( m_buttonsRight[1].m_hWnd != NULL )
	{
		rect.right = rect.Width() + cx2 + 3 + 20;
		rect.left = cx2 +3 + 20;
		m_buttonsRight[1].MoveWindow( rect );
		m_buttonsRight[1].SizeToContent();
	}
	if ( m_buttonsRight[2].m_hWnd != NULL )
	{
		rect.right = rect.Width() + cx2 + 3 + 20 + 20;
		rect.left = cx2 + 3 + 20 + 20;
		m_buttonsRight[2].MoveWindow( rect );
		m_buttonsRight[2].SizeToContent();
	}

	// path edit
//	rect.top = 2;
	rect.top = 0;
//	rect.bottom = rect.top + 15;
	rect.bottom = rect.top + 275;
	rect.left = 0;
	CRect rectR( rect );
	rect.left = ( m_bSaveMore ? 60 : 20 );
	rect.right = cx2 - 2;
	m_comboDirLeft.MoveWindow( rect );
	rectR = rect;
	rectR.right = rectR.Width() + cx2 + rect.left + 2;
	rectR.left = cx2 + rect.left + 2;
	m_comboDirRight.MoveWindow( rectR );

	// listbox or tree
	rect.left = 0;
	rect.top = 22;
	rect.bottom = cy;
	rect.right = cx;
	MoveClient( rect );

	SelectSide2( s_nSide );
}

BOOL CViewFileSync::Show( BOOL b )
{
	TRACE2( "CViewFileSync::Show( %d ) %s\n", b, CString(GetRuntimeClass()->m_lpszClassName) );
	if ( b )
	{
		SetDlgCtrlID( AFX_IDW_PANE_FIRST );		// activate for CFrameWnd::RecalcLayout
		ShowWindow(SW_SHOW);
	}
	else
	{
		// save documents ...
		if ( !SaveModified( common ) )
			return FALSE;
//		if ( m_pDoc[0] != NULL )
//		{
//			if ( !m_pDoc[0]->SaveModified(0) )
//				return FALSE;
//		}
//		if ( m_pDoc[1] != NULL )
//		{
//			if ( !m_pDoc[1]->SaveModified(1) )
//				return FALSE;
//		}
		DeleteContents();	// erase view before documents
		if ( m_pDoc[0] != NULL )
		{
			if ( !m_pDoc[0]->ResetAll() )
				return FALSE;
		}
		if ( m_pDoc[1] != NULL )
		{
			if ( !m_pDoc[1]->ResetAll() )
				return FALSE;
		}
		SetDlgCtrlID( m_nMenueID );		// deactivate for CFrameWnd::RecalcLayout
		ShowWindow(SW_HIDE);
	}
	return TRUE;
}

void CViewFileSync::DeleteContents()
{
	TRACE0( "CViewFileSync::DeleteContents()\n" );
	m_comboDirLeft.SetWindowText( _T("") );
	m_comboDirRight.SetWindowText( _T("") );
	m_strOldPath[0].Empty();
	m_strOldPath[1].Empty();
	changeNotify[0].MonitorEnd();
	changeNotify[1].MonitorEnd();
	m_bChanged[0] = FALSE;
	m_bChanged[1] = FALSE;
//	GetParentFrame()->SetIndicator( ID_INDICATOR_LEFT, FALSE );
//	GetParentFrame()->SetIndicator( ID_INDICATOR_RIGHT, FALSE );
	m_buttonsLeft[1].LoadBitmapsEx(0U);	// disable
	m_buttonsRight[1].LoadBitmapsEx(0U);	
}

void CViewFileSync::RemoveListMode()
{
	if ( m_pListMode != NULL )
	{
		m_pListMode->DestroyWindow();
		delete m_pListMode;
		m_pListMode = NULL;
	}
}


void CViewFileSync::OnFileOpenDirRight()
{
	TRACE0("CViewFileSync::OnFileOpenDirRight ====================== \n");
	if ( !SaveModified( common ) )
		return;
	s_nSide = right;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[1]->GetBasePathName();
	CString str = pDM->OnDirOpen(m_pDoc[1]->GetPIDL());
//	StoreViewDir( str, _T("PathRight") );
}

void CViewFileSync::OnFileOpenDirLeft()
{
	TRACE0("CViewFileSync::OnFileOpenDirLeft ====================== \n");
	if ( !SaveModified( common ) )
		return;
	s_nSide = left;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[0]->GetBasePathName();
	CString str = pDM->OnDirOpen(m_pDoc[0]->GetPIDL());
//	StoreViewDir( str, _T("PathLeft") );
}

void CViewFileSync::OnFileOpenRight()
{
	TRACE0("CViewFileSync::OnFileOpenRight ====================== \n");
	if ( !SaveModified( common ) )
		return;
	s_nSide = right;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[1]->GetBasePathName();
	pDM->SetType( pDM->FindTemplate( m_nTitleID ) );
	pDM->OnFileOpen();
}

void CViewFileSync::OnFileOpenLeft()
{
	TRACE0("CViewFileSync::OnFileOpenLeft ====================== \n");
	if ( !SaveModified( common ) )
		return;
	s_nSide = left;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[0]->GetBasePathName();
	pDM->SetType( pDM->FindTemplate( m_nTitleID ) );
	pDM->OnFileOpen();
}

void CViewFileSync::OnUpdateViewRefresh(CCmdUI *pCmdUI)
{
//	pCmdUI->Enable( m_bChanged[0] ||
//					m_bChanged[1] ||
//					m_pDoc[left]->IsModified() ||
//					m_pDoc[right]->IsModified() );
	pCmdUI->Enable( TRUE );
	//CToolBar &toolBar = GetParentFrame()->GetToolBar();
	int nButton = pCmdUI->m_nIndex;
	int nButtonLast = pCmdUI->m_nIndexMax - 1;
	if ( pCmdUI->m_pOther != NULL && nButton >= 0 ) {
		CToolBar *pToolBar = (CToolBar*)pCmdUI->m_pOther;
		UINT nID;
		UINT nStyle;
		int nImage;
		pToolBar->GetButtonInfo(nButtonLast, nID, nStyle, nImage);
		++nImage; // green
		if ( m_bChanged[0] ||
			 m_bChanged[1] ||
			 m_pDoc[left]->IsModified() ||
			 m_pDoc[right]->IsModified() )
			nImage -= 3; // yellow
		pToolBar->SetButtonInfo(nButton, ID_VIEW_REFRESH, TBBS_BUTTON, nImage );
	}
}

void CViewFileSync::OnViewRefresh()
{
	OnRefresh();
	SelectSide( left );	// 2010/02/18
}

BOOL CViewFileSync::OnRefresh()
{
	TRACE0("CViewFileSync::OnRefresh() =============================\n");
	if ( !SaveModified( common ) )
		return FALSE;
//	CWaitCursor wait;
	m_pProgressMan->SetCaption( _T("FileSync Progress") );
	m_pProgressMan->ResetUserAbortFlag();
//	m_pProgressMan->EnableCancel( TRUE );
//	m_pProgressMan->SetAbortText( _T("Canceled ...") );
	m_pProgressMan->SetStaticText( 0, _T("Reloading") );
	m_pProgressMan->SetStaticText( 1, _T("left") );
	m_pProgressMan->EnableProgress( TRUE );
	m_pProgressMan->SetProgress( 0 );
	__int64 nRange = 0;
	if ( m_pDoc[0] != NULL ) 
		nRange = m_pDoc[0]->GetFileSize();
	if ( m_pDoc[1] != NULL ) 
		nRange += m_pDoc[1]->GetFileSize();
	m_pProgressMan->SetRange( nRange );
	m_pProgressMan->SetVisible( TRUE );

	if ( m_pDoc[0] != NULL ) {
		m_pProgressMan->SetStaticText( 1, _T("left") );
		if ( !m_pDoc[0]->Refresh( 0, &CopyProgressRoutine, m_pProgressMan ) ) {
			m_pProgressMan->SetVisible( FALSE );
			return FALSE;
		}
	}
	if ( m_pDoc[1] != NULL ) {
		m_pProgressMan->SetProgressToBase();
		m_pProgressMan->SetStaticText( 1, _T("right") );
		if ( !m_pDoc[1]->Refresh( 1, &CopyProgressRoutine, m_pProgressMan ) ) {
			m_pProgressMan->SetVisible( FALSE );
			return FALSE;
		}
	}
	BOOL b = CompareView();
	ASSERT( b );
	m_bChanged[0] = FALSE;
	m_bChanged[1] = FALSE;
//	GetParentFrame()->SetIndicator( ID_INDICATOR_LEFT, FALSE );
//	GetParentFrame()->SetIndicator( ID_INDICATOR_RIGHT, FALSE );
	GetParentFrame()->SetMessageText( _T("Ready") );
	return TRUE;
}

void CViewFileSync::StartProgress( __int64 nRange )
{
	m_pProgressMan->SetCaption( _T("FileSync Progress") );
	m_pProgressMan->ResetUserAbortFlag();
//	m_pProgressMan->EnableCancel( TRUE );
//	m_pProgressMan->SetAbortText( _T("Canceled ...") );
	m_pProgressMan->SetStaticText( 0, _T("Loading") );
	if ( s_nSide == left )
		m_pProgressMan->SetStaticText( 1, _T("left") );
	else
		m_pProgressMan->SetStaticText( 1, _T("right") );
	m_pProgressMan->EnableProgress( TRUE );
	m_pProgressMan->SetProgress( 0 );
	m_pProgressMan->SetRange( nRange );
	m_pProgressMan->SetVisible( TRUE );
}

BOOL CViewFileSync::OnIdle(LONG lCount)
{
	CString strMsg = _T("Ready");
	if ( GetParentFrame()->UpdateMessageText( strMsg ) )
		return TRUE;

	if ( m_buttonsLeft[2].m_hWnd != NULL )
	{						// enable save buttons
		m_buttonsLeft[2].EnableWindow( m_pDoc[left]->IsModified() );
		m_buttonsRight[2].EnableWindow( m_pDoc[right]->IsModified() );
	}
	for ( int s = 0; s < 2; ++s )
	{
		LONG w = changeNotify[s].WhatIsChanged();
		if ( (w & 2) != 0 ) // RO
		{
			if ( m_pDoc[s] != NULL ) {
				m_pDoc[s]->RefreshAttr();
				if ( s == 0 ) {
					if ( m_buttonsLeft[1].GetDlgCtrlID() == ID_FILE_MODELEFT )
					{
						m_buttonsLeft[1].LoadBitmapsEx(GetModeBitmap(left, TRUE));
						m_buttonsLeft[1].Invalidate();
					}
				}
				else {
					if ( m_buttonsRight[1].GetDlgCtrlID() == ID_FILE_MODERIGHT )
					{
						m_buttonsRight[1].LoadBitmapsEx(GetModeBitmap(right, TRUE));
						m_buttonsRight[1].Invalidate();
					}
				}
			}
		}
		if ( (w & 1) != 0 )	// size or date
		{
			m_bChanged[s] = TRUE;
//			GetParentFrame()->SetIndicator( s == 0 ? ID_INDICATOR_LEFT : ID_INDICATOR_RIGHT, TRUE );
			CString strMsg = ( s == 0 ? _T("Left ") : _T("Right ") );
			strMsg += _T("area has been changed.\nRefresh view?");
			GetParentFrame()->SetMessageText( strMsg );
//			if ( MessageBox( strMsg, NULL, MB_YESNO | MB_ICONQUESTION ) == IDYES )
//			{
//				changeNotify[s].Enable( FALSE );
//				PostMessage( WM_COMMAND, ID_VIEW_REFRESH | (BN_CLICKED << 16), NULL );
//			}
			if ( s == 0 && m_buttonsLeft[1].GetDlgCtrlID() == ID_FILE_MODELEFT) {
				m_buttonsLeft[1].SetDlgCtrlID(ID_VIEW_CHECKLEFT);
				m_buttonsLeft[1].LoadBitmapsEx(IDB_CHECK);
				m_buttonsLeft[1].Invalidate();
			}
			else if (m_buttonsRight[1].GetDlgCtrlID() == ID_FILE_MODERIGHT) {
				m_buttonsRight[1].SetDlgCtrlID(ID_VIEW_CHECKRIGHT);
				m_buttonsRight[1].LoadBitmapsEx(IDB_CHECK);
				m_buttonsRight[1].Invalidate();
			}
			return TRUE;
		}
	}
	return FALSE;
}

void CViewFileSync::OnUpdateFileSaveLeft(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pDoc[left]->IsModified() );
}

void CViewFileSync::OnFileSaveLeft()
{
	OnFileSave( left );
}

void CViewFileSync::OnUpdateFileSaveRight(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pDoc[right]->IsModified() );
}

void CViewFileSync::OnFileSaveRight()
{
	OnFileSave( right );
}

void CViewFileSync::OnFileSave( Side s )
{
	TRACE0("CViewFileSync::OnFileSave ====================== \n");
	changeNotify[s].Enable( FALSE );
	m_pDoc[s]->DoFileSave();
	Sleep(0);
	m_bChanged[s] = FALSE;
	changeNotify[s].Enable();
	UpdateEditDir(s);
	UpdateModeButtons();
	m_undoBuffer.CleanModifiedFlag( s );
	SetFocus();
}

//void CViewFileSync::OnFileMoreLeft()
//{
//	CRect rect;
//	m_buttonMoreLeft.GetWindowRect( rect );
//	CMenu menu;
//	VERIFY(menu.LoadMenu(IDR_MENU_MORE));
//	CMenu* pPopup = menu.GetSubMenu(0);
//	ASSERT(pPopup != NULL);
//	s_nSide = left;
//	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.right, rect.top, GetParentFrame());
//}
//
//void CViewFileSync::OnFileMoreRight()
//{
//	CRect rect;
//	m_buttonMoreRight.GetWindowRect( rect );
//	CMenu menu;
//	VERIFY(menu.LoadMenu(IDR_MENU_MORE));
//	CMenu* pPopup = menu.GetSubMenu(0);
//	ASSERT(pPopup != NULL);
//	s_nSide = right;
//	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.right, rect.top, GetParentFrame());
//}

void CViewFileSync::OnUpdateViewDircomp(CCmdUI *pCmdUI)
{
	if ( m_pProgressMan != NULL && m_pProgressMan->GetVisible() ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	CViewFileSync *pView = pDM->GetViewDir();
	pCmdUI->Enable( pView != NULL );
}

BOOL CViewFileSync::SaveModified( Side s )
{
	if ( s == common ) {
		if ( !SaveModified( left ) )
			return FALSE;
		return SaveModified( right );
	}
	else
	{
		if ( m_pDoc[s] == NULL )
			return TRUE;
		changeNotify[s].Enable( FALSE );
		BOOL b = m_pDoc[s]->SaveModified( s );
		changeNotify[s].Enable();
		if ( b )
			m_bChanged[s] = FALSE;
		return b;
	}
}

void CViewFileSync::OnViewDircomp()
{
	TRACE0("CViewFileSync::OnViewDircomp ====================== \n");
	if ( !SaveModified( common ) )
		return;
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->OpenDirs( m_pDoc[0], m_pDoc[1] );
	m_pDoc[0]->DeleteContents();
	m_pDoc[1]->DeleteContents();

}

void CViewFileSync::ChangeDirs( const CString &strPathLeft, const CString &strPathRight )
{
	ASSERT( FALSE );
}

void CViewFileSync::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	if ( s_nSide == left )
		OnUpdateFileSaveLeft( pCmdUI );
	else
		OnUpdateFileSaveRight( pCmdUI );
}

void CViewFileSync::OnFileSave()
{
	if ( s_nSide == left )
		OnFileSaveLeft();
	else
		OnFileSaveRight();
}

void CViewFileSync::OnFileSaveLeftAs()
{
	TRACE0("CViewFileSync::OnFileSaveLeftAs ====================== \n");
	if ( !m_pDoc[left]->DoSave(NULL) )
	{
		TRACE0("Warning: Left file save-as failed.\n");
		return;
	}
	UpdateEditDir(left);
	UpdateModeButtons();
	m_undoBuffer.CleanModifiedFlag( left );
}

void CViewFileSync::OnFileSaveRightAs()
{
	TRACE0("CViewFileSync::OnFileSaveRightAs ====================== \n");
	if ( !m_pDoc[right]->DoSave(NULL) )
	{
		TRACE0("Warning: Right file save-as failed.\n");
		return;
	}
	UpdateEditDir(right);
	UpdateModeButtons();
	m_undoBuffer.CleanModifiedFlag( right );
}

void CViewFileSync::OnFileSaveAs()
{
	if ( s_nSide == left )
	{
		OnFileSaveLeftAs();
	}
	else
	{
		OnFileSaveRightAs();
	}
}

void CViewFileSync::OnFileOpen()
{
	TRACE0("CViewFileSync::OnFileOpen ====================== \n");
	if ( s_nSide == left )
		PostMessage( WM_COMMAND, ID_FILE_OPEN_LEFT | (BN_CLICKED << 16), NULL );
	else
		PostMessage( WM_COMMAND, ID_FILE_OPEN_RIGHT | (BN_CLICKED << 16), NULL );
}

void CViewFileSync::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( !m_pDoc[s_nSide]->GetPathName().IsEmpty() );
}

void CViewFileSync::OnFileClose()
{
	TRACE0("CViewFileSync::OnFileClose ====================== \n");
	if ( !SaveModified( s_nSide ) )
		return;
	if ( !m_pDoc[s_nSide]->ResetAll() )
		return;

	if ( s_nSide == left )
		m_comboDirLeft.SetWindowText( _T("") );
	else
		m_comboDirRight.SetWindowText( _T("") );
	m_strOldPath[s_nSide].Empty();
	BOOL b = CompareView();
	ASSERT( b );
//	SelectSide( s_nSide );	// 2010/02/18
}

void CViewFileSync::OnViewAssociations()
{
	TRACE0("CViewFileSync::OnViewAssociations ====================== \n");
	CString str;
	if ( s_nSide == left )
		m_comboDirLeft.GetWindowText(str);
	else
		m_comboDirRight.GetWindowText( str );
	CAssocDlg dlg;
	dlg.m_strInitPath = str;
	dlg.DoModal();
}

void CViewFileSync::OnViewColors()
{
	TRACE0("CViewFileSync::OnViewColors ====================== \n");
	CColorsDlg dlg;
	dlg.GetProfile( m_strColorsSection );
	dlg.m_acr[0] = m_crMarkDir;
	dlg.m_acr[1] = m_crMarkLite;
	dlg.m_acr[2] = m_crMark;
	dlg.m_acr[3] = m_crTxtChanged;
	dlg.m_acr[4] = m_crMarkSingle;

	if ( dlg.DoModal() == IDOK )
	{
		m_crMarkDir = dlg.m_acr[0];
		m_crMarkLite = dlg.m_acr[1];
		m_crMark = dlg.m_acr[2];
		m_crTxtChanged = dlg.m_acr[3];
		m_crMarkSingle = dlg.m_acr[4];
		Invalidate();
	}
}

void CViewFileSync::CompareEnable( BOOL bEnable /* = TRUE */ )
{
	m_bCompareEnabled = bEnable;

	if ( !m_bCompareEnabled )
		return;

	BOOL b = CompareView();
	ASSERT( b );
	SelectSide(left);	// 2010/02/18
}

CMainFrame* CViewFileSync::GetParentFrame()
{
	return (CMainFrame*)CView::GetParentFrame();
}


BOOL CViewFileSync::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CWnd *pWnd = GetFocus();
	HWND hWnd = pWnd != NULL ? pWnd->m_hWnd : NULL;
	if ( hWnd != NULL && hWnd != m_hWnd )
	{
		if ( hWnd == m_comboDirLeft.GetEditWnd() )
		{
			if ( m_comboDirLeft.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
				return TRUE;

			if ( nCode == CN_UPDATE_COMMAND_UI && CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
			{
//				((CCmdUI*)pExtra)->Enable( FALSE );
				return TRUE;
			}
		}

		if ( hWnd == m_comboDirRight.GetEditWnd() )
		{
			if ( m_comboDirRight.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
				return TRUE;

			if ( nCode == CN_UPDATE_COMMAND_UI && CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
			{
//				((CCmdUI*)pExtra)->Enable( FALSE );
				return TRUE;
			}
		}
	}

	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CViewFileSync::OnDeactivateApp()
{
	m_comboDirLeft.OnKillFocus( this );
	m_comboDirRight.OnKillFocus( this );
	RemoveListMode();
}

void CViewFileSync::OnKillFocus(CWnd* pNewWnd)
{
	// 090518
	HWND hWnd = pNewWnd != NULL ? pNewWnd->m_hWnd : NULL;
	if ( hWnd != NULL && hWnd == m_comboDirLeft.GetEditWnd() )
		s_nSide = left;
	else if ( hWnd != NULL && hWnd == m_comboDirRight.GetEditWnd() )
		s_nSide = right;

	CView::OnKillFocus(pNewWnd);

	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}


void CViewFileSync::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);

	SelectSide(s_nSide);	// highlight only
}


void CViewFileSync::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if ( !bShow )
		OnDeactivateApp();

	CView::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
}

BOOL CViewFileSync::OnToolTipNotify(UINT id, NMHDR *pNMHDR,
   LRESULT *pResult)
{
//	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMHDR;
//	int control_id =  ::GetDlgCtrlID((HWND)pNMHDR->idFrom);
//	if(control_id)
//	{
//		pText->lpszText = MAKEINTRESOURCE(control_id);
		// lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
		// todo: if text contains '|' copy last part only
//		pText->hinst = AfxGetInstanceHandle();
//		return TRUE;
//	}
//	return FALSE;

	ENSURE_ARG(pNMHDR != NULL);
	ENSURE_ARG(pResult != NULL);
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	TCHAR szFullText[256];
	CString strTipText;
	UINT_PTR nID = pNMHDR->idFrom;
	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = (UINT)::GetDlgCtrlID((HWND)nID);
	}

	if (nID == ID_FILE_MODELEFT || nID == ID_FILE_MODERIGHT) {
		UINT nBitmap = 0;
		if (nID == ID_FILE_MODELEFT)
			nBitmap = m_buttonsLeft[1].GetBitmapNo();
		else 
			nBitmap = m_buttonsRight[1].GetBitmapNo();
		nID = nBitmap;
	}
	
	if (nID != 0 && strTipText.IsEmpty()) // nID will be zero on a separator
	{
		// don't handle the message if no string resource found
		if (AfxLoadString((UINT)nID, szFullText) == 0)
			return FALSE;

		// this is the command id, not the button index
		AfxExtractSubString(strTipText, szFullText, 1, '\n');
	}
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		Checked::strncpy_s(pTTTA->szText, _countof(pTTTA->szText), strTipText, _TRUNCATE);
	else
		_mbstowcsz(pTTTW->szText, strTipText, _countof(pTTTW->szText));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, _countof(pTTTA->szText));
	else
		Checked::wcsncpy_s(pTTTW->szText, _countof(pTTTW->szText), strTipText, _TRUNCATE);
#endif
	*pResult = 0;

	// bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);

	return TRUE;    // message was handled
}

void CViewFileSync::OnFileModeleft()
{
	OnFileMode(left);
}

void CViewFileSync::OnFileModeright()
{
	OnFileMode(right);
}

void CViewFileSync::OnFileMode(int nSide)
{
	TRACE0("CViewFileSync::OnFileMode ====================== \n");
	if ( s_nSide != nSide )
		SelectSide( nSide );
	CToolButton* pButton = nSide == left ? &m_buttonsLeft[1] : &m_buttonsRight[1];
	UINT nBitmap = pButton->GetBitmapNo();
	RemoveListMode();
	m_pListMode = new CListBoxMode();
	CRect rectList;
	pButton->GetWindowRect(rectList);
	rectList.right += 100;
	rectList.bottom += 60;
	ScreenToClient(rectList);
	m_pListMode->Create( WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_OWNERDRAWFIXED, 
		rectList, this, IDC_LISTMODE );
	m_pListMode->SetFont( &m_fontEdit );
	m_pListMode->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW );
	for (int nRes = IDB_MODE_R; nRes <= IDB_MODE_G; ++nRes )
	{
//		TCHAR szFullText[256];
//		CString strText;

//		AfxLoadString((UINT)n, szFullText);
//		AfxExtractSubString(strText, szFullText, 1, '\n');

//		m_pListMode->AddString(strText);
		int n = m_pListMode->AddString(NULL);
		m_pListMode->SetItemData( n, nRes );
	}
	m_pListMode->SetCurSel( nBitmap - IDB_MODE_R );
	m_pListMode->SetFocus();
	m_pListMode->SetCapture();
}

void CViewFileSync::OnListModeDblClk()
{
	TRACE0("CViewFileSync::OnListModeDblClk ====================== \n");
	CToolButton* pButton = s_nSide == left ? &m_buttonsLeft[1] : &m_buttonsRight[1];
//	UINT nBitmap = pButton->GetBitmapNo();
	int nSel = m_pListMode->GetCurSel() + IDB_MODE_R;
	changeNotify[s_nSide].Enable( FALSE );
	switch ( nSel ) 
	{
	case IDB_MODE_R:
		m_pDoc[s_nSide]->SetReadOnly( TRUE );
		m_pDoc[s_nSide]->SetReadOnlyFile( TRUE );
		break;
	case IDB_MODE_RY:
		m_pDoc[s_nSide]->SetReadOnly( FALSE );
		m_pDoc[s_nSide]->SetReadOnlyFile( TRUE );
		break;
	case IDB_MODE_GY:
		m_pDoc[s_nSide]->SetReadOnly( TRUE );
		m_pDoc[s_nSide]->SetReadOnlyFile( FALSE );
		break;
	case IDB_MODE_G:
		m_pDoc[s_nSide]->SetReadOnly( FALSE );
		m_pDoc[s_nSide]->SetReadOnlyFile( FALSE );
		break;
	default:
		break;
	}
	pButton->LoadBitmapsEx( GetModeBitmap(s_nSide, TRUE) );
	pButton->Invalidate();
	RemoveListMode();
	Sleep(0);
	m_bChanged[s_nSide] = FALSE;
	changeNotify[s_nSide].Enable();
}

void CViewFileSync::OnViewCheckleft()
{
	if ( !OnRefresh() )
		return;
	m_buttonsLeft[1].SetDlgCtrlID(ID_FILE_MODELEFT);
	m_buttonsLeft[1].LoadBitmapsEx(IDB_MODE_G);
	m_buttonsLeft[1].Invalidate();
}


void CViewFileSync::OnViewCheckright()
{
	if ( !OnRefresh() )
		return;
	m_buttonsRight[1].SetDlgCtrlID(ID_FILE_MODERIGHT);
	m_buttonsRight[1].LoadBitmapsEx(IDB_MODE_G);
	m_buttonsRight[1].Invalidate();
}


void CViewFileSync::OnUpdateFileModeleft(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pDoc[0] != NULL );
}

void CViewFileSync::OnUpdateFileModeright(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pDoc[1] != NULL  );
}

void CViewFileSync::OnUpdateViewCheckleft(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_buttonsLeft[1].GetDlgCtrlID() == ID_VIEW_CHECKLEFT);
}


void CViewFileSync::OnUpdateViewCheckright(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_buttonsRight[1].GetDlgCtrlID() == ID_VIEW_CHECKRIGHT);
}

UINT CViewFileSync::GetModeBitmap(int nSide, BOOL bYellow)
{
	if ( m_pDoc[nSide] == NULL )
		return 0U;

	BOOL bLocal = !m_pDoc[nSide]->IsReadOnly();
	BOOL bFile = !m_pDoc[nSide]->IsReadOnlyFile();
	if ( bFile && bLocal )
		return IDB_MODE_G;
	if ( bFile && bYellow )
		return IDB_MODE_GY;
	if ( bLocal && bYellow )
		return IDB_MODE_RY;
	return IDB_MODE_R;
}


BOOL CViewFileSync::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ( ((LPNMHDR)lParam)->code == LBN_DBLCLK && 
		 ((LPNMHDR)lParam)->idFrom == IDC_LISTMODE )
	{
		OnListModeDblClk();
		return TRUE;
	}
	return CView::OnNotify(wParam, lParam, pResult);
}

void CViewFileSync::UpdateModeButtons() {
	if ( m_buttonsLeft[1].GetDlgCtrlID() == ID_FILE_MODELEFT )
	{
		m_buttonsLeft[1].LoadBitmapsEx(GetModeBitmap(left, TRUE));
		m_buttonsLeft[1].Invalidate();
	}
	if ( m_buttonsRight[1].GetDlgCtrlID() == ID_FILE_MODERIGHT )
	{
		m_buttonsRight[1].LoadBitmapsEx(GetModeBitmap(right, TRUE));
		m_buttonsRight[1].Invalidate();
	}
}

BOOL CViewFileSync::GetMainIcon(HICON &hIcon, BOOL bBigIcon)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	int nPix = bBigIcon ? 32 : 16;
	HICON hIconNew = (HICON)LoadImage( hInst, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, nPix, nPix, LR_SHARED);
//	HICON hIconNew = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	if ( hIconNew == hIcon )
		return FALSE;

	hIcon = hIconNew;
	return TRUE;
}

void CViewFileSync::OnViewText()
{
	OnViewChangeTo( IDR_VIEWTEXT );
}

void CViewFileSync::OnViewXml()
{
	OnViewChangeTo( IDR_VIEWXML );
}


void CViewFileSync::OnViewHex()
{
	OnViewChangeTo( IDR_VIEWHEX );
}

void CViewFileSync::OnViewChangeTo(UINT nIDResource)
{
	TRACE0("CViewFileSync::OnViewChangeTo()\n");
	if ( !m_pDoc[0]->SaveModified( 0 ) )
		return;
	if ( !m_pDoc[1]->SaveModified( 1 ) )
		return;
	CWaitCursor waitCursor;
	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	CDocTemplFileSync *pTemplate = pDM->FindTemplate( nIDResource );
	pDM->SetType( pTemplate );
	CDocDir *pDocDirL = m_pDoc[left]->GetParentDoc();
	POSITION posL = m_pDoc[left]->GetParentPos();
	CString strFullPathExL = m_pDoc[left]->GetPathName();
	CDocDir *pDocDirR = m_pDoc[right]->GetParentDoc();
	POSITION posR = m_pDoc[right]->GetParentPos();
	CString strFullPathExR = m_pDoc[right]->GetPathName();

	CompareEnable( FALSE );

	SetSide( left );
	CDocFileSync *pDocL = (CDocFileSync *)pDM->OpenDocumentFile( strFullPathExL, pTemplate );
	if ( pDocL != NULL && posL != NULL )
	{
		pDocL->SetParentAndPos( pDocDirL, posL );
	}

	SetSide( right );
	CDocFileSync *pDocR = (CDocFileSync *)pDM->OpenDocumentFile( strFullPathExR, pTemplate );
	if ( pDocR != NULL && posR != NULL )
	{
		pDocR->SetParentAndPos( pDocDirR, posR );
	}
	
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
	pView->UpdateEditDir(0);
	pView->UpdateEditDir(1);
	pView->CompareEnable( TRUE );
}

void CViewFileSync::OnHelpContents()
{
	CFileSyncApp* pApp = (CFileSyncApp*)AfxGetApp();
	ShellExecute( GetParentFrame()->m_hWnd, _T("open"), _T("index.html"), NULL, pApp->m_strHelpPath, SW_SHOW );
}

