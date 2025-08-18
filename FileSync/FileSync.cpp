// FileSync.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DocManFileSync.h"
#include "DocTemplFileSync.h"
#include "DocHex.h"
//#include "DocText.h"
#include "DocXml.h"
#include "DocDirNative.h"
#include "DocDirZipRoot.h"
#include "DocDirIsoRoot.h"
#include "ViewHex.h"
//#include "ViewText.h"
#include "ViewXml.h"
#include "ViewDir.h"
#include "ChangeNotification.h"
#include "MainFrm.h"

#include "FileSync.h"

#pragma comment( lib, "version" )

// CFileSyncApp

BEGIN_MESSAGE_MAP(CFileSyncApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CFileSyncApp construction

CFileSyncApp::CFileSyncApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CFileSyncApp object

CFileSyncApp theApp;

// CFileSyncApp initialization

BOOL CFileSyncApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

//	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART; manager not found & bad cmd line

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
#ifdef _DEBUG
	SetRegistryKey(_T("Peter Pagel Debug"));
#else
	SetRegistryKey(_T("Peter Pagel"));
#endif

	TCHAR szFullPath[510];
	GetModuleFileName( NULL, szFullPath, sizeof(szFullPath) );
	TCHAR *pc = _tcsrchr(szFullPath, '\\');
#pragma warning( push )
#pragma warning( disable : 4996 )
	_tcscpy(pc + 1, _T("html"));
#pragma warning( pop ) 
	m_strHelpPath = GetProfileString(_T("Help"), _T("Path"), szFullPath);

	m_pDocManager = new CDocManFileSync();
	CDocTemplFileSync* pDocTemplate;

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWHEX,
		RUNTIME_CLASS(CDocHex),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewHex));
	pDocTemplate->SetDefault();
	pDocTemplate->SetIconNo( 2 );
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWZIP,
		RUNTIME_CLASS(CDocDirZipRoot),		
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewDir));			
	pDocTemplate->GetProfile( _T("ZIP"), "zip\0jar\0war\0ear\0rar\0sar\0odt\0ods\0" );
	pDocTemplate->SetIconNo( 3 );
	AddDocTemplate(pDocTemplate);
	CDocTemplFileSync* pDocTemplateZip = pDocTemplate;

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWISO,
		RUNTIME_CLASS(CDocDirIsoRoot),		
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewDir));				
	pDocTemplate->GetProfile( _T("ISO"), "iso\0" );
	pDocTemplate->SetIconNo( 6 );
	AddDocTemplate(pDocTemplate);
	CDocTemplFileSync* pDocTemplateIso = pDocTemplate;

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWTEXT,
		RUNTIME_CLASS(CDocText),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewText));
	pDocTemplate->GetProfile( _T("TEXT"), "asm\0bas\0bat\0bsh\0c\0cmd\0config\0cpp\0css\0dsp\0dsw\0dtd\0frm\0h\0htm\0html\0inc\0inf\0ini\0inl\0log\0java\0jsp\0manifest\0mf\0prn\0policy\0properties\0props\0reg\0rc\0save\0sh\0sln\0sql\0tld\0vbp\0vbs\0vbw\0vcproj\0txt\0xmi\0xsd\0xsl\0" );
	pDocTemplate->SetIconNo( 4 );
	AddDocTemplate(pDocTemplate);
	CDocTemplFileSync* pDocTemplateText = pDocTemplate;

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWXML,
		RUNTIME_CLASS(CDocXml),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewXml));
	pDocTemplate->GetProfile( _T("XML"), "xml\0" );
	pDocTemplate->SetIconNo( 5 );
	AddDocTemplate(pDocTemplate);

	// to add a new template:
	// DocFileSync.h: enum Icons
	// ViewFileSync.cpp OnCreate(): m_imagelist
	// Bitmap IDB_TREEIMAGES
	// String IDR_VIEWxxx
	// Icon IDR_VIEWxxx (???)
	// Menu IDR_MENU_TYPE
	// ViewDir.cpp ON_COMMAND(ID_TYPE_xxx,...)

	pDocTemplate = new CDocTemplFileSync(
		IDR_VIEWDIR,
		RUNTIME_CLASS(CDocDirNative),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CViewDir));
//	pDocTemplate->GetProfile( _T("DIR") );
	pDocTemplate->SetIconNo( 1 );
	AddDocTemplate(pDocTemplate);

	CArcRoot ar;
	ar.CleanTemp();

    CString csCmd = m_lpCmdLine;
	CStringArray csArgs;
	SplitCmdLine(csCmd, csArgs);
	if (csArgs.GetCount() < 1)
	{	// default
		pDocTemplate->OpenDocumentFile( NULL );
	//	if (!((CMainFrame*)m_pMainWnd)->RestoreWinPos())
	//		m_pMainWnd->ShowWindow(SW_SHOW); // fallback if failed
	} else { // single arg or MKS 2/4 args
		CViewFileSync::SetSide(CViewFileSync::left);
		CDocFileSync *pDocL = (CDocFileSync*) pDocTemplateText->OpenDocumentFile( csArgs.GetAt(0) );
		CDocFileSync *pDocR;
		if ( csArgs.GetCount() >= 2 ) {
			CViewFileSync::SetSide(CViewFileSync::right);
			pDocR = (CDocFileSync*) pDocTemplateText->OpenDocumentFile( csArgs.GetAt(1) );
		}
		if ( csArgs.GetCount() >= 4 ) {
			m_pMainWnd->SetWindowText( _T("FileSync ") + csArgs.GetAt(2) + _T(" - ") + csArgs.GetAt(3) );
			pDocL->SetReadOnly(TRUE);
			pDocR->SetReadOnly(TRUE);
			CViewFileSync* pView = (CViewFileSync*)((CMainFrame*)m_pMainWnd)->GetActiveView();
			pView->UpdateModeButtons();
		}
	}

	pDocTemplateZip->SetView( pDocTemplate->GetView() );	// use same view object

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
//	CMainFrame* pFrame = new CMainFrame;
//	m_pMainWnd = pFrame;
	// create and load the frame with its resources
//	pFrame->LoadFrame(IDR_MAINFRAME,
//		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
//		NULL);
	// The one and only window has been initialized, so show and update it
//	pFrame->ShowWindow(SW_SHOW);
//	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

BOOL CFileSyncApp::ExitInstance()
{
	CChangeNotification::Cleanup();
	return CWinApp::ExitInstance();
}

// CFileSyncApp message handlers

void CFileSyncApp::SplitCmdLine( const CString& csCmd, CStringArray &csArgs )
{
	int p = 0;
	int q = -1;
	if (csCmd.IsEmpty())
		return;

	while ( p >= 0 && p < csCmd.GetLength() ) {
		if ( csCmd.GetAt(p) == '"' ) {
			++p;
			q = csCmd.Find('"', p);
		} else {
			q = csCmd.Find(' ', p+1);
		}
		if ( q < 0 )
			q = csCmd.GetLength();
		if ( q <= p ) 
			break;
		csArgs.Add( csCmd.Mid(p, q-p) );
		if ( csCmd.GetAt(q) == '"' )
			q = csCmd.Find(' ', q+1);
		if ( q < 0 )
			break;
		p = q+1;
	}
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strVersion( _T("FileSync Beta Version ") );

//	CString strExe = AfxGetApp()->m_pszHelpFilePath;
//	strExe = strExe.Left(strExe.GetLength()-3) + _T("exe");
//	LPTSTR szFullPath = (LPTSTR)(LPCTSTR)strExe;
	TCHAR szFullPath[MAX_PATH];
	GetModuleFileName( AfxGetApp()->m_hInstance, szFullPath, sizeof(szFullPath) );
	DWORD dwVerHnd;
	DWORD dwVerInfoSize =
		GetFileVersionInfoSize(szFullPath, &dwVerHnd);

	if (dwVerInfoSize)
	{
		BOOL  fRet;
		// BOOL fRetName;
		LPSTR lpstrVffInfo; /* Pointer to block to hold info */
		LPTSTR lszVer = NULL;
		//LPTSTR lszVerName = NULL;
		UINT  cchVer = 0;

		lpstrVffInfo  = new char[dwVerInfoSize];

		/* Get the version information */
		GetFileVersionInfo(szFullPath, 0L, dwVerInfoSize, lpstrVffInfo);
		fRet = VerQueryValue(lpstrVffInfo,
				  TEXT("\\StringFileInfo\\040904E4\\FileVersion"),
				   (LPVOID *)&lszVer,
				   &cchVer);
		//fRetName = VerQueryValue(lpstrVffInfo,
		//		   TEXT("\\StringFileInfo\\040904E4\\CompanyName"),
		//		  (LPVOID *)&lszVerName,
		//		  &cchVer);
		if ( fRet )
			strVersion += lszVer;
		delete [] lpstrVffInfo;
	}
	GetDlgItem(IDC_STATIC_VERSION)->SetWindowText( strVersion );
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CFileSyncApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CFileSyncApp message handlers


BOOL CFileSyncApp::OnIdle(LONG lCount)
{
	BOOL bMore = CWinApp::OnIdle(lCount);
	if ( !bMore && m_pMainWnd != NULL )
	{
		bMore = ((CMainFrame *)m_pMainWnd)->OnIdle( lCount );
	}
	return bMore;
}

