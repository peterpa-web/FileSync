// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FileSync.h"
#include "DocTemplFileSync.h"
#include "ViewFileSync.h"
#include "DocManFileSync.h"
//#include "ViewDir.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
//	ON_COMMAND(ID_VIEW_DIRCOMP, OnViewDircomp)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
//	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LEFT, OnUpdateIndicator)
//	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RIGHT, OnUpdateIndicator)
	ON_WM_ACTIVATEAPP()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnToolbarReset)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_PROGRESS,	// progress bar
//	ID_INDICATOR_LEFT,
//	ID_INDICATOR_RIGHT,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

const int nStatusInfo = 0;
const int nStatusProgress = 1;

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nToolbarID = 0;
//	m_pViewDir = NULL;
	m_bIndicatorLeft = FALSE;
	m_bIndicatorRight = FALSE;
	m_bActive = TRUE;
}

CMainFrame::~CMainFrame()
{
	AfxGetApp()->m_pMainWnd = NULL;

	POSITION pos = m_mapToolBarCache.GetStartPosition();
	while (pos != NULL)
	{
		int n;
		CToolBarFixed* pToolBar;
		m_mapToolBarCache.GetNextAssoc(pos, n, pToolBar);
		delete pToolBar;
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

//	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
//		| /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
//		CRect(), IDR_VIEWDIR) ||
//		!m_wndToolBar.LoadToolBar(IDR_VIEWDIR))
//	{
//		TRACE0("Failed to create toolbar\n");
//		return -1;      // fail to create
//	}

	if (!m_wndToolBarSearch.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
		CRect(), IDR_TOOL_SEARCH) ||
		!m_wndToolBarSearch.LoadToolBar(IDR_TOOL_SEARCH))
	{
		TRACE0("Failed to create search toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetPaneWidth (nStatusProgress, 80);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarSearch.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
//	DockPane(&m_wndToolBarSearch);
//	DockPaneLeftOf(&m_wndToolBar, &m_wndToolBarSearch);
	SwitchToolBar(IDR_VIEWDIR);

	m_progressMan.Init( GetSafeHwnd() );
	m_progressMan.SetVisible( FALSE );
	m_progressMan.BeginProgressDialog();

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext)
{
	// default create client will create a view if asked for it
	if (pContext != NULL && pContext->m_pNewViewClass != NULL)
	{
		CWnd *pView = CreateView(pContext, AFX_IDW_PANE_FIRST);
		m_pNewClient = (CViewFileSync*)pView;
		if (pView == NULL)
			return FALSE;
	}
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	HICON hIcon = ::LoadIcon( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME) );
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass( 0, 0, 0, hIcon );
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	CView* pView = GetActiveView();
	if ( pView != NULL )
		pView->SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	CViewFileSync *pView = (CViewFileSync*)GetActiveView();
	if ( pView != NULL && pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
		return TRUE;

	// otherwise, do default handling
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainFrame::OnIdle(LONG lCount)
{
//	return m_wndView.OnIdle( lCount );
	CView *pView = GetActiveView();
	if ( pView == NULL )
		return FALSE;
	if ( ((CViewFileSync*)pView)->GetCurrToolbarID() != m_nToolbarID )
	{
		int nToolbarID = ((CViewFileSync*)pView)->GetCurrToolbarID();
		SwitchToolBar(nToolbarID);
//		m_wndToolBar.LoadBitmap( m_nToolbarID );
//		m_wndToolBar.Invalidate();
//		DockPane(&m_wndToolBarSearch);
//		DockPaneLeftOf(&m_wndToolBar, &m_wndToolBarSearch);

	}
	BOOL bIdle = ((CViewFileSync*)pView)->OnIdle( lCount );
	return bIdle;
}

BOOL CMainFrame::UpdateMessageText( const CString &strMsg, int nProgress /* = 0 */ )
{
	if ( strMsg.Compare( m_strMsg ) == 0 )
		return FALSE;

	if ( nProgress != 0 ) {
		m_wndStatusBar.EnablePaneProgressBar(nStatusProgress);
		m_wndStatusBar.SetPaneProgress(nStatusProgress, nProgress);
	}
	else
		m_wndStatusBar.EnablePaneProgressBar(nStatusProgress, -1);

	m_strMsg = strMsg;
	SetMessageText( m_strMsg );
	TRACE1( "UpdateMessageText %s\n", m_strMsg );
	return TRUE;
}

void CMainFrame::OnClose()
{
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	CViewFileSync* pDirView = pDM->GetViewDir();
	CViewFileSync* pView = (CViewFileSync*)GetActiveView();

	if ( pDirView != NULL && pDirView != pView ) {
		int nRC = MessageBox( _T("Exit Application?"), NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
		if ( nRC != IDYES ) {
			PostMessage( WM_COMMAND, ID_VIEW_DIRCOMP | (BN_CLICKED << 16), NULL );
			return; // don't close from file view without request
		}
//		Beep( 100, 100 );
	}

	if ( !pView->Show(FALSE) )	// check busy / save documents ... may fail
		return;
	pView->DeleteContents();

	if ( pDirView != NULL ) {
		SaveWinPos();
		pDirView->SaveHistory();
		if ( !pDirView->Show(FALSE) )	// save documents ... may fail
			return;
		pDirView->DeleteContents();
	}
	TRACE0("call CFrameWndEx::OnClose\n");
	CFrameWndEx::OnClose();
}

void CMainFrame::SaveWinPos()
{
	WINDOWPLACEMENT wndpl;
	if ( GetWindowPlacement(&wndpl) ) {
		if (wndpl.showCmd == SW_SHOWNORMAL ||
			wndpl.showCmd == SW_SHOWMAXIMIZED) {
			HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("MainFrm") );
			ASSERT( hSecKey != NULL );
			if (hSecKey == NULL)
				return;

			CRect rect(wndpl.rcNormalPosition);
			DWORD dwX = rect.left;
			DWORD dwY = rect.top;
			DWORD dwCX = rect.Width();
			DWORD dwCY = rect.Height();
			DWORD dwShow = wndpl.showCmd;
			LONG lResult;
			lResult = RegSetValueEx(hSecKey, _T("x"), NULL, REG_DWORD, (LPBYTE)&dwX, sizeof(DWORD));
			lResult = RegSetValueEx(hSecKey, _T("y"), NULL, REG_DWORD, (LPBYTE)&dwY, sizeof(DWORD));
			lResult = RegSetValueEx(hSecKey, _T("cx"), NULL, REG_DWORD, (LPBYTE)&dwCX, sizeof(DWORD));
			lResult = RegSetValueEx(hSecKey, _T("cy"), NULL, REG_DWORD, (LPBYTE)&dwCY, sizeof(DWORD));
			lResult = RegSetValueEx(hSecKey, _T("show"), NULL, REG_DWORD, (LPBYTE)&dwShow, sizeof(DWORD));
			RegCloseKey(hSecKey);
		}
	}
}

void CMainFrame::SwitchToolBar(int nToolbarID)
{
	if (nToolbarID == m_nToolbarID)
		return;

	if (m_nToolbarID != 0)
	{
		CToolBarFixed* pOldToolBar;
		if (m_mapToolBarCache.Lookup(m_nToolbarID, pOldToolBar))
			pOldToolBar->ShowWindow(SW_HIDE);
	}
	CToolBarFixed* pToolBar = nullptr;
	if (m_mapToolBarCache.Lookup(nToolbarID, pToolBar))
		pToolBar->ShowWindow(SW_SHOW);
	else
	{
		pToolBar = new CToolBarFixed;
		VERIFY(pToolBar->CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(), nToolbarID));
		VERIFY(pToolBar->LoadToolBar(nToolbarID));
		m_mapToolBarCache.SetAt(nToolbarID, pToolBar);
		pToolBar->EnableDocking(CBRS_ALIGN_ANY);
	}
	DockPane(&m_wndToolBarSearch);
	DockPaneLeftOf(pToolBar, &m_wndToolBarSearch);
	m_nToolbarID = nToolbarID;
}

BOOL CMainFrame::RestoreWinPos()
{
	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("MainFrm") );
	ASSERT( hSecKey != NULL );
	if (hSecKey == NULL)
		return FALSE;

	DWORD x, y, cx, cy, show, dwSize;
	LONG lResult;
	lResult = RegQueryValueEx(hSecKey,  _T("x"), NULL, NULL, (LPBYTE)&x, &dwSize );
	if ( lResult == ERROR_SUCCESS )
		lResult = RegQueryValueEx(hSecKey,  _T("y"), NULL, NULL, (LPBYTE)&y, &dwSize );
	if ( lResult == ERROR_SUCCESS )
		lResult = RegQueryValueEx(hSecKey,  _T("cx"), NULL, NULL, (LPBYTE)&cx, &dwSize );
	if ( lResult == ERROR_SUCCESS )
		lResult = RegQueryValueEx(hSecKey,  _T("cy"), NULL, NULL, (LPBYTE)&cy, &dwSize );
	if ( lResult == ERROR_SUCCESS )
		lResult = RegQueryValueEx(hSecKey,  _T("show"), NULL, NULL, (LPBYTE)&show, &dwSize );
	if ( lResult == ERROR_SUCCESS ) {
		CRect rect(x, y, x+cx, y+cy);
		HMONITOR hMon = MonitorFromRect( rect, MONITOR_DEFAULTTONULL );
			if ( hMon != NULL ) {
				SetWindowPos( NULL, x, y, cx, cy, SWP_NOZORDER );
				ShowWindow(show);
			}
			else
				ShowWindow(SW_SHOW);
	}
	RegCloseKey(hSecKey);
	return (lResult == ERROR_SUCCESS);
}

void CMainFrame::SetActiveView( CView *pViewNew, BOOL bNotify /* = TRUE */ )
{
	TRACE1( "CMainFrame::SetActiveView( %s )\n", CString(pViewNew->GetRuntimeClass()->m_lpszClassName) );
	CViewFileSync* pOldView = (CViewFileSync*)GetActiveView();
	CViewFileSync *pViewNewFS = (CViewFileSync *)pViewNew;
	if ( pOldView != pViewNew )
	{
		if ( !pOldView->Show(FALSE) )	// save documents ... may fail
			return;
		pViewNewFS->Show(TRUE);
	}
	TRACE0( "CMainFrame::SetActiveView LoadToolBar & Menue\n" );
//	VERIFY( m_wndToolBar.LoadToolBar( pViewNewFS->GetMenueID() ) );
	RecalcLayout( TRUE );
	SwitchToolBar(pViewNewFS->GetMenueID());
//	DockPane(&m_wndToolBarSearch);
//	DockPaneLeftOf(&m_wndToolBar, &m_wndToolBarSearch);

	SetMenu( pViewNewFS->GetMenu() );
	HICON hBigIcon = GetIcon(TRUE);
	if (pViewNewFS->GetMainIcon(hBigIcon, TRUE))
		SetIcon( hBigIcon, TRUE );
	HICON hIcon = GetIcon(FALSE);
	if (pViewNewFS->GetMainIcon(hIcon, FALSE))
		SetIcon( hIcon, FALSE );
	CString strTitle;
	strTitle.LoadString( pViewNewFS->GetTitleID() );
	SetWindowText( strTitle );
	CFrameWndEx::SetActiveView( pViewNew );
	RecalcLayout();
}

//void CMainFrame::OnUpdateViewDircomp(CCmdUI *pCmdUI)
//{
//	pCmdUI->SetCheck((m_pDirCompFrmWnd->GetStyle() & WS_VISIBLE) != 0);
//}

//void CMainFrame::OnViewDircomp()
//{
//	ASSERT( m_pViewDir != NULL );
//	SetActiveView( m_pViewDir );
//}

void CMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle , CWnd* pParentWnd , CCreateContext* pContext)
{
	if ( (dwDefaultStyle & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW )
		return CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);

	// TODO: create new view
/*
	TRACE1( "CMainFrame::LoadFrame() view %s\n", pContext->m_pNewViewClass->m_lpszClassName );
	CViewFileSync *pView = (CViewFileSync*) CreateView( pContext );
	//  pContext->m_pNewViewClass->CreateObject();
	//CDocTemplFileSync *pDocTempl = (CDocTemplFileSync *)pContext->m_pNewDocTemplate;
	//if (!pView->Create(NULL, NULL, WS_CHILD,
	//	CRect(0, 0, 0, 0), this, pDocTempl->GetResourceID(), pContext))
	if ( pView == NULL )
	{
		TRACE0("Failed to create view\n");
		return FALSE;
	}
	SetActiveView( pView );

	*/
	return TRUE;
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	POINT ptMinTrackSize = { 500, 200 };
	lpMMI->ptMinTrackSize = ptMinTrackSize;

	CFrameWndEx::OnGetMinMaxInfo(lpMMI);
}

//void CMainFrame::OnUpdateIndicator(CCmdUI* pCmdUI)
//{
//	BOOL b;
//	switch (pCmdUI->m_nID)
//	{
//	case ID_INDICATOR_LEFT:
//		b = m_bIndicatorLeft;
//		break;

//	case ID_INDICATOR_RIGHT:
//		b = m_bIndicatorRight;
//		break;

///	default:
//		TRACE1( "Warning: OnUpdateIndicator - unknown indicator 0x%04X.\n",
//			pCmdUI->m_nID);
//		pCmdUI->ContinueRouting();
//		return; // not for us
//	}

//	pCmdUI->Enable(b);
//	ASSERT(pCmdUI->m_bEnableChanged);
//}

//void CMainFrame::SetIndicator( int nId, BOOL b )
//{
//	if ( nId == ID_INDICATOR_LEFT )
//		m_bIndicatorLeft = b;

//	else if ( nId == ID_INDICATOR_RIGHT )
//		m_bIndicatorRight = b;
//}

//BOOL CMainFrame::GetIndicator( int nId )
//{
//	if ( nId == ID_INDICATOR_LEFT )
//		return m_bIndicatorLeft;

//	return m_bIndicatorRight;
//}



void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CFrameWndEx::OnActivateApp(bActive, dwThreadID);

	m_bActive = bActive;
	CViewFileSync *pView = (CViewFileSync*)GetActiveView();
	if ( !bActive && pView != NULL )
		pView->OnDeactivateApp();
}

LRESULT CMainFrame::OnToolbarReset(WPARAM wparm, LPARAM)
{
	UINT uitoolbarid = (UINT)wparm;
	if (uitoolbarid == IDR_TOOL_SEARCH) {
		m_wndToolBarSearch.Reset();
	}

	return 0;
}