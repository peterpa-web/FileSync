// REBProgressDialog.cpp : implementation file
// http://www.codeguru.com/cpp/w-d/dislog/progressindicators/article.php/c1999/

#include "StdAfx.h"
#include "rebprogressdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void REBInitializeProgressData(REBPROGRESSDATA* pData)
{
	// Set defaults
	ASSERT(pData!=NULL);
	::_tcscpy_s(pData->cCaption,       REBPROGRESSMAXSTRING, _T("Progress Dialog") );
	::_tcscpy_s(pData->cAbortText,     REBPROGRESSMAXSTRING, _T("Aborting process, please wait...") );
	::_tcscpy_s(pData->cStaticText[0], REBPROGRESSMAXSTRING, _T("In Process") );
	::_tcscpy_s(pData->cStaticText[1], REBPROGRESSMAXSTRING, _T("") );
	::_tcscpy_s(pData->cStaticText[2], REBPROGRESSMAXSTRING, _T("0 s") );
	pData->bCancelEnabled =	         FALSE;
	pData->bProgressEnabled =        FALSE;
	pData->bVisible =                TRUE;
	pData->nProgress =               0;
}

void REBCopyProgressData(REBPROGRESSDATA* pDataDest, REBPROGRESSDATA* pDataSource)
{
	// Copy data in pDataSource to pDataDest
	ASSERT(pDataDest && pDataSource);
	::_tcscpy_s(pDataDest->cCaption,   REBPROGRESSMAXSTRING, pDataSource->cCaption);
	::_tcscpy_s(pDataDest->cAbortText, REBPROGRESSMAXSTRING, pDataSource->cAbortText);
	for(int i=0; i < REBPROGRESSMAXSTATIC; i++)
	{
		::_tcscpy_s(pDataDest->cStaticText[i], REBPROGRESSMAXSTRING, pDataSource->cStaticText[i]);
	}
	pDataDest->bCancelEnabled =   pDataSource->bCancelEnabled;
	pDataDest->bProgressEnabled = pDataSource->bProgressEnabled;
	pDataDest->bVisible =         pDataSource->bVisible;
	pDataDest->nProgress =        pDataSource->nProgress;
}

REBPROGRESSDATA    g_ProgressData; // global structure holding progress data
BOOL               g_bUserAbortFlag = FALSE; // did user abort?
CRITICAL_SECTION   g_Crit1; // protects g_ProgressData structure
CRITICAL_SECTION   g_Crit2; // protects g_bUserAbortFlag
HWND               g_hwndProgress = NULL;
UINT               REBREFRESHPROGRESS = ::RegisterWindowMessage(_T("REBRefreshProgress"));
DWORD              g_dwTime = 0;

#define REBPRGRS_PUMPINTERVAL 2000

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// REBProgressManager - runs the progress dialog

REBProgressManager::REBProgressManager()
{
	m_bInited = FALSE;
	m_pThread = NULL;
	g_dwTime = ::GetTickCount();
}

REBProgressManager::~REBProgressManager()
{
	if(m_bInited)
	{
		Exit();
	}
	ASSERT(m_pThread == NULL);
}

void REBProgressManager::CheckAndPump()
{
	// This is a modified YieldProc type function - it gets called often, but only
	// pumps the message que when REBPRGRS_PUMPINTERVAL has elapsed.
	DWORD dwNow = ::GetTickCount();
	if((dwNow - g_dwTime) >= REBPRGRS_PUMPINTERVAL)
	{
		g_dwTime = dwNow;
#ifdef _DEBUG
		_AFX_THREAD_STATE *pState = AfxGetThreadState();
#endif
		// pump message que
		MSG msg;
		while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
#ifdef _DEBUG
			if (pState->m_nDisablePumpCount != 0)
				return;
#endif
			if(!AfxGetApp()->PumpMessage())
			{
				::PostQuitMessage(0);
			}
		}
//		LONG lIdle = 0;							// 20100819
//		while(AfxGetApp()->OnIdle(lIdle++));
	}
}

void REBProgressManager::Init(HWND hwndParent)
{
	m_hwndParent = hwndParent;
	ASSERT(::IsWindow(m_hwndParent));
	ASSERT(REBREFRESHPROGRESS != 0);
	REBInitializeProgressData(&g_ProgressData);
	m_bVisible = g_ProgressData.bVisible;
	m_bInited = TRUE;
}

void REBProgressManager::Exit()
{
	m_bInited = FALSE;
	if(m_pThread != NULL)
	{
		EndProgressDialog();
	}
}

void REBProgressManager::BeginProgressDialog()
{
	ASSERT(m_pThread == NULL);
	ASSERT(m_hwndParent && ::IsWindow(m_hwndParent));

	g_bUserAbortFlag = FALSE;

	::InitializeCriticalSection(&g_Crit1);
	::InitializeCriticalSection(&g_Crit2);
	m_pThread = new REBProgressThread(m_hwndParent);
	m_pThread->m_bAutoDelete = FALSE;
	VERIFY( m_pThread->CreateThread() );

	// REB - apparently, you have to show the window for the first time from outside the thread it was
	// created it in, or things want to hang.  The same thing with destroying the window (see EndProgressDialog).
//	WaitForProgressDialog(); // wait while UI thread creates dialog
//	::ShowWindow(g_hwndProgress, (m_bVisible ? SW_SHOW : SW_HIDE)); // show window for the first time
}

void REBProgressManager::EndProgressDialog()
{
	ASSERT(m_pThread!=NULL);

	// Destroy the progress dialog
	ASSERT(g_hwndProgress!=NULL);
	::SendMessage(g_hwndProgress, WM_DESTROY, 0, 0);
	while(::IsWindow(g_hwndProgress))
	{
		::Sleep(0); // wait until it is destroyed
	}
	g_hwndProgress = NULL;

	DWORD dwWait = WAIT_FAILED;
	while ( dwWait != WAIT_OBJECT_0 ) {
		::PostThreadMessage(m_pThread->m_nThreadID, WM_QUIT, 0, 0);
		dwWait = ::WaitForSingleObject(m_pThread->m_hThread, 200);
		TRACE1( "REBProgressManager::EndProgressDialog() %d\n", dwWait );
	}
	delete m_pThread;
	m_pThread = NULL;
	::DeleteCriticalSection(&g_Crit2);
	::DeleteCriticalSection(&g_Crit1);

	REBInitializeProgressData(&g_ProgressData); // reinitialize data in case progress dialog is used again
	m_bVisible = g_ProgressData.bVisible;
}

void REBProgressManager::PreAccess(int nCritSec)
{
	ASSERT(nCritSec == 1 || nCritSec == 2);
	if(m_pThread != NULL)
	{
		::EnterCriticalSection(((nCritSec == 1) ? &g_Crit1 : &g_Crit2));
	}
}

void REBProgressManager::PostAccess(int nCritSec)
{
	ASSERT(nCritSec == 1 || nCritSec == 2);
	if(m_pThread != NULL)
	{
		::LeaveCriticalSection(((nCritSec == 1) ? &g_Crit1 : &g_Crit2));
	}
	CheckAndPump();
}

void REBProgressManager::NotifyChange()
{
	if(m_pThread != NULL && g_hwndProgress != NULL)
	{
		WaitForProgressDialog();
		::SendMessage(g_hwndProgress, REBREFRESHPROGRESS, 0, 0);
	}
}

void REBProgressManager::WaitForProgressDialog()
{
	if(m_pThread != NULL)
	{
		// if our thread has just started, it may not have had time yet to initialize the dialog window
		if(g_hwndProgress == NULL)
		{
			while(g_hwndProgress == NULL)
			{
				::Sleep(0); // give up THIS thread's time slice to allow the other thread to finish its InitInstance
			}
		}
		if(!::IsWindow(g_hwndProgress))
		{
			while(!::IsWindow(g_hwndProgress))
			{
				::Sleep(0); // give up THIS thread's time slice to allow the other thread to finish its InitInstance
			}
		}
	}
}

void REBProgressManager::ResetUserAbortFlag()
{
	ASSERT(m_bInited);

	PreAccess(2);
	g_bUserAbortFlag = FALSE;
	PostAccess(2);
}

BOOL REBProgressManager::GetUserAbortFlag()
{
	ASSERT(m_bInited);

	BOOL bRet = FALSE;
	PreAccess(2);
	bRet = g_bUserAbortFlag;
	PostAccess(2);
	return bRet;
}

LPBOOL REBProgressManager::GetUserAbortFlagPtr()
{
	return &g_bUserAbortFlag;
}

LPCTSTR REBProgressManager::GetAbortText()
{
	ASSERT(m_bInited);
	LPCTSTR pszRet = NULL;
	PreAccess(1);
	pszRet = g_ProgressData.cAbortText;
	PostAccess(1);
	return pszRet;
}

void REBProgressManager::SetAbortText(LPCTSTR pszText)
{
	ASSERT(m_bInited);
	ASSERT(::_tcslen(pszText) <= REBPROGRESSMAXSTRING);
	PreAccess(1);
	::_tcscpy_s(g_ProgressData.cAbortText, REBPROGRESSMAXSTRING, pszText);
	PostAccess(1);
	NotifyChange();
}

BOOL REBProgressManager::IsProgressEnabled()
{
	ASSERT(m_bInited);
	BOOL bRet = FALSE;
	PreAccess(1);
	bRet = g_ProgressData.bProgressEnabled;
	PostAccess(1);
	return bRet;
}

BOOL REBProgressManager::IsCancelEnabled()
{
	ASSERT(m_bInited);
	BOOL bRet = FALSE;
	PreAccess(1);
	bRet = g_ProgressData.bCancelEnabled;
	PostAccess(1);
	return bRet;
}

void REBProgressManager::EnableProgress(BOOL bEnable)
{
	ASSERT(m_bInited);
	PreAccess(1);
	g_ProgressData.bProgressEnabled = bEnable;
	PostAccess(1);
	NotifyChange();
}

void REBProgressManager::EnableCancel(BOOL bEnable)
{
	ASSERT(m_bInited);
	PreAccess(1);
	g_ProgressData.bCancelEnabled = bEnable;
	PostAccess(1);
	NotifyChange();
}

int REBProgressManager::GetProgress()
{
	ASSERT(m_bInited);
	int nProg = 0;
	PreAccess(1);
	nProg = g_ProgressData.nProgress;
	PostAccess(1);
	return nProg;
}

void REBProgressManager::SetProgress(int nVal)
{
	ASSERT(m_bInited);
	CString strTimeRem;
	if ( m_nProgr == nVal ) 
		return;

	m_nProgr = nVal;		// 0..100
	if ( nVal == 0 )
		m_dwTickStart = GetTickCount();
	else {
		DWORD dwTickDelta = (GetTickCount() - m_dwTickStart) / 1000;		// seconds consumed until now 
		dwTickDelta = (100 - m_nProgr) * dwTickDelta / m_nProgr + 1;
		if ( dwTickDelta >= 60 )
//			strTimeRem.Format( _T("%d min %d s"), dwTickDelta / 60, dwTickDelta % 60 );
			strTimeRem.Format( _T("%d min"), (dwTickDelta+30) / 60 );
		else if ( dwTickDelta > 10 )
			strTimeRem.Format( _T("%d s"), 5 * (dwTickDelta/5) );
		else
			strTimeRem.Format( _T("%d s"), dwTickDelta );
	}
	PreAccess(1);
	g_ProgressData.nProgress = nVal;
	::_tcscpy_s(g_ProgressData.cStaticText[2], REBPROGRESSMAXSTRING, strTimeRem);
	PostAccess(1);
	NotifyChange();
}

void REBProgressManager::SetRange(__int64 nVal)
{
	if ( nVal < 1 )
		nVal = 100;
	m_nRange = nVal;
	m_nProgress = 0;
	m_nBase = 0;
	m_nProgr = -1;
	SetProgress( 0 );
}

void REBProgressManager::AddProgress(__int64 nVal)
{
	m_nProgress += nVal;
	SetProgress();
}

void REBProgressManager::SetProgress()
{
	// using m_nProgress
	int n = 0;
	if ( m_nRange < 1000 )
		n = (int)(m_nProgress * 100 / m_nRange);
	else
		n = (int)(m_nProgress / (m_nRange / 100));
	SetProgress( n );
}

void REBProgressManager::SetProgressToBase()
{
	m_nBase = m_nProgress;
}

void REBProgressManager::AddProgressBased(__int64 nVal)
{
	m_nProgress = m_nBase + nVal;
	SetProgress();
}

void REBProgressManager::SetVisible(BOOL bNewVal)
{
	ASSERT(m_bInited);
	PreAccess(1);
	g_ProgressData.bVisible = bNewVal;
	PostAccess(1);
	m_bVisible = bNewVal;
	NotifyChange();
}

BOOL REBProgressManager::GetVisible()
{
	ASSERT(m_bInited);
	BOOL bRet = FALSE;
	PreAccess(1);
	bRet = g_ProgressData.bVisible;
	PostAccess(1);
	return bRet;
}

LPCTSTR REBProgressManager::GetCaption()
{
	ASSERT(m_bInited);
	LPCTSTR pszRet = NULL;
	PreAccess(1);
	pszRet = g_ProgressData.cCaption;
	PostAccess(1);
	return pszRet;
}

void REBProgressManager::SetCaption(LPCTSTR pszCaption)
{
	ASSERT(m_bInited);
	ASSERT(::_tcslen(pszCaption) <= REBPROGRESSMAXSTRING);
	PreAccess(1);
	::_tcscpy_s(g_ProgressData.cCaption, REBPROGRESSMAXSTRING, pszCaption);
	PostAccess(1);
	NotifyChange();
}

LPCTSTR REBProgressManager::GetStaticText(int nIndex)
{
	ASSERT(m_bInited);
	BOOL bValidIndex = (nIndex > -1 && nIndex < REBPROGRESSMAXSTATIC);
	ASSERT(bValidIndex);
	if(!bValidIndex)
	{
		return NULL;
	}

	LPCTSTR pszReturn = NULL;
	PreAccess(1);
	pszReturn = g_ProgressData.cStaticText[nIndex];
	PostAccess(1);
	return pszReturn;
}

void REBProgressManager::SetStaticText(int nIndex, LPCTSTR pszText)
{
	ASSERT(m_bInited);
//	ASSERT(::_tcslen(pszText) <= REBPROGRESSMAXSTRING);
	BOOL bValidIndex = (nIndex > -1 && nIndex < REBPROGRESSMAXSTATIC);
	ASSERT(bValidIndex);
	if(!bValidIndex)
	{
		return;
	}

	PreAccess(1);
	::_tcsncpy_s(g_ProgressData.cStaticText[nIndex], REBPROGRESSMAXSTRING, pszText, REBPROGRESSMAXSTRING-1);
	PostAccess(1);
	NotifyChange();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// REBProgressThread - the UI thread that runs the progress dialog

IMPLEMENT_DYNCREATE(REBProgressThread, CWinThread);

REBProgressThread::REBProgressThread(HWND hwndParent) :
	m_hwndParent(hwndParent)
{
}

REBProgressThread::REBProgressThread() : // constructor required by DECLARE_DYNCREATE macro (not used)
	m_hwndParent(NULL)
{
}

REBProgressThread::~REBProgressThread()
{
}

BOOL REBProgressThread::InitInstance()
{
	// NOTE: the memory allocated below is freed by REBProgressDialog::OnDestroy (it deletes itself)
	REBProgressDialog* pDlg = new REBProgressDialog(CWnd::FromHandle(m_hwndParent));
	VERIFY( pDlg->Create(IDD_DIALOG_PROGRESS, CWnd::FromHandle(m_hwndParent)) );
	g_hwndProgress = pDlg->GetSafeHwnd();
	TRACE0("REBProgressThread::InitInstance()\n");
	return TRUE;
}

int REBProgressThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// REBProgressDialog dialog

REBProgressDialog::REBProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(REBProgressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(REBProgressDialog)
	m_csText1 = _T("");
	m_csText2 = _T("");
	m_csText3 = _T("");
	//}}AFX_DATA_INIT

//	m_bUserAbortFlag = FALSE;
	m_bCancelEnabled = TRUE;
	m_bProgressEnabled = TRUE;
	m_nPos = 0;
}


void REBProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(REBProgressDialog)
	DDX_Control(pDX, IDCANCEL, m_buttonCancel);
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_ctrlProgress);
	DDX_Text(pDX, IDC_STATIC_TEXT1, m_csText1);
	DDX_Text(pDX, IDC_STATIC_TEXT2, m_csText2);
	DDX_Text(pDX, IDC_STATIC_TEXT3, m_csText3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(REBProgressDialog, CDialog)
	//{{AFX_MSG_MAP(REBProgressDialog)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(REBREFRESHPROGRESS,OnRefresh)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// REBProgressDialog message handlers

BOOL REBProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	Reposition();

	m_ctrlProgress.SetRange32(0, 100);
	RefreshData(TRUE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void REBProgressDialog::Reposition()
{
	// REB - do not use CenterWindow (this locks things up)
	HWND hwndParent = ::GetParent(GetSafeHwnd());
	ASSERT(hwndParent!=NULL);
	CRect rect, rectParent;
	::GetWindowRect(GetSafeHwnd(), &rect);
	::GetWindowRect(hwndParent, &rectParent);
	BOOL bOK = ((rectParent.Height() > rect.Height()) && (rectParent.Width() > rect.Width()));
	if(!bOK)
	{
		// if parent window is too small, center on desktop
		::GetWindowRect(::GetDesktopWindow(), &rectParent);
	}
	int nLeft = (rectParent.left + (rectParent.Width() / 2)) - (rect.Width() / 2);
	int nTop = (rectParent.top + (rectParent.Height() / 2)) - (rect.Height() / 2);
	::MoveWindow(GetSafeHwnd(), nLeft, nTop, rect.Width(), rect.Height(), FALSE);
}

void REBProgressDialog::OnCancel()
{
//	m_bUserAbortFlag = TRUE;
	::EnterCriticalSection(&g_Crit2);
	g_bUserAbortFlag = TRUE;
	::LeaveCriticalSection(&g_Crit2);
	HandleUserAbort();
}

void REBProgressDialog::OnDestroy()
{
	CDialog::OnDestroy();
	delete this; // destroy our C++ object
}

void REBProgressDialog::HandleUserAbort()
{
//	ASSERT(m_bUserAbortFlag == TRUE);
	UpdateData(TRUE);
	m_buttonCancel.EnableWindow(FALSE);
	m_buttonCancel.ShowWindow(SW_HIDE);
	m_ctrlProgress.EnableWindow(FALSE);
	m_ctrlProgress.ShowWindow(SW_HIDE);
	m_csText1 = m_csAbortText;
	m_csText2.Empty();
	m_csText3.Empty();
	m_nPos = 0;
	m_bCancelEnabled = FALSE;
	m_bProgressEnabled = FALSE;
	UpdateData(FALSE);
	UpdateWindow();
}

LRESULT REBProgressDialog::OnRefresh(WPARAM wParam, LPARAM lParam)
{
	RefreshData();
	return 0;
}

void REBProgressDialog::RefreshData(BOOL bInit /*= FALSE*/)
{
//	if(!m_bUserAbortFlag)
	if(!g_bUserAbortFlag)
	{
		::EnterCriticalSection(&g_Crit1);
		::REBCopyProgressData(&m_TempData, &g_ProgressData);
		::LeaveCriticalSection(&g_Crit1);

		BOOL bUpdateWindow = FALSE;
		m_csAbortText = m_TempData.cAbortText;
		m_csText1 = m_TempData.cStaticText[0];
		m_csText2 = m_TempData.cStaticText[1];
		m_csText3 = m_TempData.cStaticText[2];
		m_nPos = m_TempData.nProgress;

		if(bInit) // TRUE when called from OnInitDialog
		{
			bUpdateWindow = TRUE;
			m_bVisible = m_TempData.bVisible;
			m_bCancelEnabled = m_TempData.bCancelEnabled;
			m_bProgressEnabled = m_TempData.bProgressEnabled;
			m_csCaption = m_TempData.cCaption;
			m_buttonCancel.EnableWindow(m_bCancelEnabled);
			m_buttonCancel.ShowWindow(m_bCancelEnabled ? SW_SHOW : SW_HIDE);
			m_ctrlProgress.EnableWindow(m_bProgressEnabled);
			m_ctrlProgress.ShowWindow(m_bProgressEnabled ? SW_SHOW : SW_HIDE);
			if(m_bProgressEnabled)
			{
				m_ctrlProgress.SetPos(m_nPos);
			}
			SetWindowText(m_csCaption);
//			ShowWindow(m_bVisible ? SW_SHOW : SW_HIDE); // REBProgressManager will call the initial ShowWindow
		}
		else
		{
			if(m_bCancelEnabled != m_TempData.bCancelEnabled)
			{
				m_bCancelEnabled = m_TempData.bCancelEnabled;
				m_buttonCancel.EnableWindow(m_bCancelEnabled);
				m_buttonCancel.ShowWindow(m_bCancelEnabled ? SW_SHOW : SW_HIDE);
				bUpdateWindow = TRUE;
			}
			if(m_bProgressEnabled != m_TempData.bProgressEnabled)
			{
				m_bProgressEnabled = m_TempData.bProgressEnabled;
				m_ctrlProgress.EnableWindow(m_bProgressEnabled);
				m_ctrlProgress.ShowWindow(m_bProgressEnabled ? SW_SHOW : SW_HIDE);
				bUpdateWindow = TRUE;
			}
			if(m_bProgressEnabled)
			{
				m_ctrlProgress.SetPos(m_nPos);
			}
			if(m_csCaption.Compare(m_TempData.cCaption) != 0)
			{
				m_csCaption = m_TempData.cCaption;
				SetWindowText(m_csCaption);
				bUpdateWindow = TRUE;
			}
			if(m_bVisible != m_TempData.bVisible)
			{
				if (m_TempData.bVisible)
					Reposition();
				m_bVisible = m_TempData.bVisible;
				ShowWindow(m_bVisible ? SW_SHOW : SW_HIDE);
				bUpdateWindow = TRUE;
			}
		}

		UpdateData(FALSE);
		if(bUpdateWindow)
		{
			UpdateWindow();
		}
	}
}

