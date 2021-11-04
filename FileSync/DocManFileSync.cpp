#include "StdAfx.h"
#include "DocTemplFileSync.h"
#include "MainFrm.h"
//#include <shlobj.h>
//#include <shlwapi.h>
//#include "ViewFileSync.h"
#include "ViewDir.h"
#include "resource.h"
#include "FileDialogExt.h"
#include "pidl.h"

#include "DocManFileSync.h"

BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AFXAPI AfxResolveShortcut(CWnd* pWnd, LPCTSTR lpszFileIn,
								LPTSTR lpszFileOut, int cchPath);

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

IMPLEMENT_DYNAMIC(CDocManFileSync, CDocManager)

CDocManFileSync::CDocManFileSync(void)
{
	m_bReadOnly = FALSE;
}

AFX_STATIC void AFXAPI _AfxAppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
		!strFilterExt.IsEmpty() &&
		pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
		!strFilterName.IsEmpty())
	{
		if (pstrDefaultExt != NULL)
			pstrDefaultExt->Empty();

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please

		int iStart = 0;
		do
		{
			// VC6
			CString strExtension;
			int ps = strFilterExt.Find(';', iStart );
			if ( ps != -1 )
			{
				strExtension = strFilterExt.Mid(iStart, ps-iStart);
				iStart = ps + 1;
			}
			else
				iStart = ps;
//			CString strExtension = strFilterExt.Tokenize( _T( ";" ), iStart );

			if (iStart != -1)
			{
				// a file based document template - add to filter list

				// If you hit the following ASSERT, your document template 
				// string is formatted incorrectly.  The section of your 
				// document template string that specifies the allowable file
				// extensions should be formatted as follows:
				//    .<ext1>;.<ext2>;.<ext3>
				// Extensions may contain wildcards (e.g. '?', '*'), but must
				// begin with a '.' and be separated from one another by a ';'.
				// Example:
				//    .jpg;.jpeg
				ASSERT(strExtension[0] == '.');
				if ((pstrDefaultExt != NULL) && pstrDefaultExt->IsEmpty())
				{
					// set the default extension
					*pstrDefaultExt = strExtension.Mid( 1 );  // skip the '.'
					ofn.lpstrDefExt = const_cast< LPTSTR >((LPCTSTR)(*pstrDefaultExt));
					ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
				}

				filter += (TCHAR)'*';
				filter += strExtension;
				filter += (TCHAR)';';  // Always append a ';'.  The last ';' will get replaced with a '\0' later.
			}
		} while (iStart != -1);

		filter.SetAt( filter.GetLength()-1, '\0' );;  // Replace the last ';' with a '\0'
		ofn.nMaxCustFilter++;
	}
}

// void CDocManager::CloseAllDocuments(BOOL bEndSession)

BOOL CDocManFileSync::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	if ( !bOpenFileDialog )
		return CDocManager::DoPromptFileName( fileName, nIDSTitle, lFlags, bOpenFileDialog, pTemplate);

	TRACE0( "CDocManFileSync::DoPromptFileName()\n" );
	CFileDialogExt dlgFile(bOpenFileDialog, NULL, NULL, /* OFN_HIDEREADONLY | */ OFN_OVERWRITEPROMPT, NULL, NULL); // VC6
//	CFileDialog dlgFile(bOpenFileDialog, NULL, NULL, /* OFN_HIDEREADONLY | */ OFN_OVERWRITEPROMPT, NULL, NULL, 0);

	dlgFile.SetTemplate( NULL, IDD_OPEN_EXT);
	dlgFile.SetType( m_strType );

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags /* | OFN_ENABLEHOOK */ | OFN_EXPLORER;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		_AfxAppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	}
	else
	{
		// do for all doc template
		POSITION pos = m_templateList.GetHeadPosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			_AfxAppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	if ( !m_strInitialDir.IsEmpty() )
		dlgFile.m_ofn.lpstrInitialDir = m_strInitialDir;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	INT_PTR nResult = dlgFile.DoModal();
	m_bReadOnly = dlgFile.GetReadOnlyPref();
	m_strType = dlgFile.GetType();
	fileName.ReleaseBuffer();
	return nResult == IDOK;
}

static LPITEMIDLIST pidlInitDir = NULL;

static int CALLBACK BrowseCallbackProc(
						HWND hwnd, 
						UINT uMsg, 
						LPARAM lParam, 
						LPARAM lpData
						)
{
	if ( uMsg == BFFM_INITIALIZED && pidlInitDir != NULL )
	{
		// -- Set the initial directory.
		SendMessage(hwnd, BFFM_SETSELECTION, FALSE, (LPARAM)pidlInitDir);
	}
	return 0;
}

LPITEMIDLIST CDocManFileSync::BrowseForFolder( HWND hWnd, LPCTSTR lpszTitle, UINT nFlags, 
										 LPCTSTR lpszInitDir, LPITEMIDLIST pidlOld )
{
	CPIDL pidlOld2;
	TCHAR szDisplayName[_MAX_PATH];

	if ( pidlOld != NULL )
	{
		pidlInitDir = pidlOld;
	} else {

		if ( lpszInitDir == NULL || *lpszInitDir == '\0' )
		{
			GetCurrentDirectory( _MAX_PATH, szDisplayName );
			lpszInitDir = szDisplayName;
		}
		pidlOld2.FromString((LPTSTR)lpszInitDir);
		pidlInitDir = pidlOld2;
	}

	BROWSEINFO browseInfo;
	browseInfo.hwndOwner = hWnd;
	browseInfo.pidlRoot = NULL; 	// set root at Desktop
	browseInfo.pszDisplayName = szDisplayName;
	browseInfo.lpszTitle = lpszTitle;   // passed in
	browseInfo.ulFlags = nFlags;   // also passed in
	browseInfo.lpfn = BrowseCallbackProc;
	browseInfo.lParam = 0;      // not used

	return ::SHBrowseForFolder(&browseInfo);
}

CString CDocManFileSync::OnDirOpen(LPITEMIDLIST pidlOld)
{
	// prompt the user (with all document templates)
	LPITEMIDLIST pidl = BrowseForFolder( AfxGetApp()->m_pMainWnd->m_hWnd, 
			_T("Select a folder - then press OK"), 
			BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS, m_strInitialDir, pidlOld );
	if ( pidl != NULL )
		OpenDocumentDir(pidl);

	return m_strInitialDir;
}

void CDocManFileSync::OpenDirs( CDocFileSync *pDocL, CDocFileSync *pDocR )
{
	TRACE0( "CDocManFileSync::OpenDirs()\n" );
//	ASSERT( !pDocL->GetPathName().IsEmpty() );
//	ASSERT( !pDocR->GetPathName().IsEmpty() );

	BOOL bNoDiff = FALSE;
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CViewFileSync *pOldView = (CViewFileSync *)pFrame->GetActiveView();
	if ( pOldView != NULL )
		bNoDiff = pOldView->IsNoDifference();	// files are equal?

	CDocTemplFileSync* pTemplate = (CDocTemplFileSync*)m_templateList.GetTail();	// find CDocDirNative
	ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
	CViewDir *pViewDir = (CViewDir*)pTemplate->GetView();
	ASSERT_KINDOF(CViewDir, pViewDir);
	pViewDir->CompareEnable( FALSE );

	CDocDir *pParentDocL = pDocL->GetParentDoc();		// safe them before SetActiveView
	CString strBasePathL = pDocL->GetBasePathName();
	CDocDir *pParentDocR = pDocR->GetParentDoc();
	CString strBasePathR = pDocR->GetBasePathName();

	pFrame->SetActiveView( pViewDir );

	if ( !pViewDir->ReactiveDocument(CViewFileSync::left, pParentDocL, strBasePathL) && !strBasePathL.IsEmpty() )
	{
		CViewFileSync::SetSide( CViewFileSync::left );
		OpenDocumentDir( strBasePathL );
	}

	if ( !pViewDir->ReactiveDocument(CViewFileSync::right, pParentDocR, strBasePathR) && !strBasePathR.IsEmpty() )
	{
		CViewFileSync::SetSide( CViewFileSync::right );
		OpenDocumentDir( strBasePathR );
	}

	pViewDir->CompareEnable( TRUE );

	if ( bNoDiff )
		pViewDir->MarkSelEqual( TRUE );
	else if ( pOldView != NULL && !strBasePathL.IsEmpty() && !strBasePathR.IsEmpty() )
		pViewDir->MarkSelEqual( FALSE );
}

CViewFileSync* CDocManFileSync::GetViewDir()
{
	return ((CDocTemplFileSync*)m_templateList.GetTail())->GetView();
}

void CDocManFileSync::OpenDirs( const CString &strDirLeft, const CString &strDirRight )
{
	TRACE2( "CDocManFileSync::OpenDirs(2) %s %s\n", strDirLeft, strDirRight );
	CViewDir *pViewDir = (CViewDir*)GetViewDir();
	ASSERT_KINDOF(CViewDir, pViewDir);
	pViewDir->CompareEnable( FALSE );

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
	CViewFileSync::SetSide( CViewFileSync::left );
	if ( !strDirLeft.IsEmpty() )
		OpenDocumentDir( strDirLeft );

	pView = (CViewFileSync *)pFrame->GetActiveView();
	CViewFileSync::SetSide( CViewFileSync::right );
	if ( !strDirRight.IsEmpty() )
		OpenDocumentDir( strDirRight );
	
	pViewDir->CompareEnable( TRUE );
}

CDocument* CDocManFileSync::OpenDocumentDir(LPITEMIDLIST pidl)
{
	m_strInitialDir = CPIDL::ToString(pidl);
	CDocDir* pDoc = (CDocDir*)OpenDocumentDir(m_strInitialDir);
	if ( pDoc != NULL )
		pDoc->SetPIDL( pidl );
	return pDoc;
}

void CDocManFileSync::ResolveShortcut( LPTSTR pszPath, LPCTSTR lpszFileName )
{
	ASSERT(lstrlen(lpszFileName) < _MAX_PATH);
	TCHAR szTemp[_MAX_PATH];
	if (lpszFileName[0] == '\"')
		++lpszFileName;
	lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;
	AfxFullPath(pszPath, szTemp);
	TCHAR szLinkName[_MAX_PATH];
	if (AfxResolveShortcut(AfxGetMainWnd(), pszPath, szLinkName, _MAX_PATH))
		lstrcpy(pszPath, szLinkName);
}

CDocTemplFileSync* CDocManFileSync::FindDirTemplate( LPTSTR lpszBasePath )
{
	// find the highest confidence
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	CDocTemplFileSync* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;

	TCHAR *pSep = lpszBasePath + _tcslen( lpszBasePath );	// start at end of string
	while ( pSep != NULL )
	{
		*pSep = '\0';
		// check for existing dir first
		struct _stati64 fs;
		int result = _tstati64( lpszBasePath, &fs );
		if ( result == 0 && (fs.st_mode & _S_IFDIR) != 0 ) {
			POSITION pos = m_templateList.GetTailPosition();	// get template for CDocDirNative
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetPrev(pos);
			ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
			pBestTemplate = (CDocTemplFileSync*)pTemplate;
			return pBestTemplate;
		}
		// check for archive types
		POSITION pos = m_templateList.GetHeadPosition();
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			ASSERT_KINDOF(CDocTemplFileSync, pTemplate);

			if ( !((CDocTemplFileSync*)pTemplate)->IsViewKindOf( RUNTIME_CLASS(CViewDir) ) )
				continue;

			CDocTemplate::Confidence match;
			ASSERT(pOpenDocument == NULL);
			match = pTemplate->MatchDocType(lpszBasePath, pOpenDocument);
			if (match > bestMatch)
			{
				bestMatch = match;
				ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
				pBestTemplate = (CDocTemplFileSync*)pTemplate;
			}
			if (match >= CDocTemplate::yesAttemptNative)
				break;      // stop here
		}
		if ( bestMatch >= CDocTemplate::yesAttemptNative )
			break;	// found
		pSep = _tcsrchr( lpszBasePath, '\\' );
	}
	if ( bestMatch < CDocTemplate::yesAttemptNative )
		return NULL;
	
	return pBestTemplate;
}

CDocFileSync* CDocManFileSync::CreateDocumentDir(LPCTSTR lpszDirName)
{
	TRACE1( "CDocManFileSync::CreateDocumentDir() %s\n", lpszDirName );

	TCHAR szPath[_MAX_PATH];
	if ( lpszDirName[0] == '\\' && lpszDirName[1] == '\\' && ! CDocDir::CheckConn(lpszDirName, FALSE) )
		_tcscpy_s( szPath, _MAX_PATH, lpszDirName );
	else
		ResolveShortcut(szPath, lpszDirName);

	TCHAR szBasePath[_MAX_PATH];
	_tcscpy_s( szBasePath, _MAX_PATH, szPath );

	CDocTemplFileSync* pBestTemplate = FindDirTemplate( szBasePath );
	if ( pBestTemplate == NULL )
		return NULL;

	CDocument *pDoc = pBestTemplate->CreateNewDocument();
	ASSERT_KINDOF(CDocFileSync, pDoc);
	if ( pDoc->IsKindOf(RUNTIME_CLASS( CDocDir ) ) )
		((CDocDir*)pDoc)->SetBasePath( szBasePath );
	return (CDocFileSync*)pDoc;
}

CDocument* CDocManFileSync::OpenDocumentDir(LPCTSTR lpszDirName)
{
	TRACE1( "CDocManFileSync::OpenDocumentDir() %s\n", lpszDirName );
	TCHAR szPath[_MAX_PATH];
	ResolveShortcut(szPath, lpszDirName);

	TCHAR szBasePath[_MAX_PATH];
	_tcscpy_s( szBasePath, _MAX_PATH, szPath );

	CDocTemplFileSync* pBestTemplate = FindDirTemplate( szBasePath );
	if ( pBestTemplate == NULL )
	{
		_tcscpy_s( szBasePath, _MAX_PATH, szPath );
		struct _stati64 fs;
		int result = _tstati64( szBasePath, &fs );
		if ( result != 0 || (fs.st_mode & _S_IFDIR) == 0 ) {
			TCHAR *pSep = _tcsrchr( szBasePath, '\\' );
			if ( pSep != NULL && !(pSep[2] == '$' && pSep[3] == 0) )		// don't cut c$ etc.
				*pSep = '\0';
			_tcscpy_s( szPath, _MAX_PATH, szBasePath );
		}
		POSITION pos = m_templateList.GetTailPosition();	// get template for CDocDirNative
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetPrev(pos);
		ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
		pBestTemplate = (CDocTemplFileSync*)pTemplate;
	}
	if (pBestTemplate == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return NULL;
	}

	CViewFileSync *pView = ((CDocTemplFileSync*)pBestTemplate)->GetView();	// 20120502 reset old view
	if ( pView != NULL ) {
		pView->ResetSide();
	}

	CDocument* pOpenDocument = pBestTemplate->OpenDocumentFile(szPath);		// 060511
	if ( pOpenDocument != NULL ) {
		ASSERT_KINDOF(CDocDir, pOpenDocument);
		((CDocDir*)pOpenDocument)->ResetPIDL();
		((CDocDir*)pOpenDocument)->SetBasePath(szBasePath);
	}
	return pOpenDocument;
}

void CDocManFileSync::OnFileOpen()
{
	// prompt the user (with all document templates)
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  /* OFN_HIDEREADONLY | */ OFN_FILEMUSTEXIST, TRUE, NULL))		// PP
		return; // open cancelled

	if ( m_strType == _T("hex") )
	{
		OpenDocumentFile(newName, FindTemplate(IDR_VIEWHEX) );
	}
	else if ( m_strType == _T("text") )
	{
		OpenDocumentFile(newName, FindTemplate(IDR_VIEWTEXT) );
	}
	else if ( m_strType == _T("xml") )
	{
		OpenDocumentFile(newName, FindTemplate(IDR_VIEWXML) );
	}
	else
	{
		m_strType.Empty();
		AfxGetApp()->OpenDocumentFile(newName);
	}
}

CDocument* CDocManFileSync::OpenDocumentFile(LPCTSTR lpszFileName )
{
	return OpenDocumentFile( lpszFileName, NULL );
}

CDocument* CDocManFileSync::OpenDocumentFile(LPCTSTR lpszFileName, CDocTemplFileSync* pBestTemplate )
{
	TRACE0( "CDocManFileSync::OpenDocumentFile()\n" );
	// find the highest confidence
	POSITION pos = m_templateList.GetHeadPosition();
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	if ( pBestTemplate != NULL )
		bestMatch = CDocTemplate::yesAttemptNative;
//	CDocTemplate* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;

	TCHAR szPath[_MAX_PATH];
//	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
//	TCHAR szTemp[_MAX_PATH];
//	if (lpszFileName[0] == '\"')
//		++lpszFileName;
//	lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
//	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
//	if (lpszLast != NULL)
//		*lpszLast = 0;
//	AfxFullPath(szPath, szTemp);
//	TCHAR szLinkName[_MAX_PATH];
//	if (AfxResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
//		lstrcpy(szPath, szLinkName);
	ResolveShortcut(szPath, lpszFileName);

	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocTemplate::Confidence match;
		ASSERT(pOpenDocument == NULL);
		match = pTemplate->MatchDocType(szPath, pOpenDocument);
		if (match > bestMatch)
		{
			bestMatch = match;
			ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
			pBestTemplate = (CDocTemplFileSync*)pTemplate;
		}
		if (match == CDocTemplate::yesAlreadyOpen)
			break;      // stop here
	}

	if (pOpenDocument != NULL)
	{
		CDocTemplFileSync *pDocTempl = (CDocTemplFileSync*)pOpenDocument->GetDocTemplate();
		CViewFileSync *pView = pDocTempl->GetView();
		ASSERT( pView != NULL );
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		CViewFileSync *pViewOld = (CViewFileSync *)pFrame->GetActiveView();
		pFrame->SetActiveView( (CView*)pView );

		return pOpenDocument;
	}

	if (pBestTemplate == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return NULL;
	}

	return pBestTemplate->OpenDocumentFile(szPath);
}

int CDocManFileSync::GetMatchingIconNo(LPCTSTR lpszFileName)
{
//	TRACE0( "CDocManFileSync::GetMatchingIconNo()\n" );
	// find the highest confidence
	POSITION pos = m_templateList.GetHeadPosition();
	CDocTemplate::Confidence bestMatch = CDocTemplate::noAttempt;
	CDocTemplFileSync* pBestTemplate = NULL;
	CDocument* pOpenDocument = NULL;

	TCHAR szPath[_MAX_PATH];
//	ASSERT(lstrlen(lpszFileName) < _countof(szPath));
//	TCHAR szTemp[_MAX_PATH];
//	if (lpszFileName[0] == '\"')
//		++lpszFileName;
//	lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
//	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
//	if (lpszLast != NULL)
//		*lpszLast = 0;
//	AfxFullPath(szPath, szTemp);
//	TCHAR szLinkName[_MAX_PATH];
//	if (AfxResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
//		lstrcpy(szPath, szLinkName);
	ResolveShortcut(szPath, lpszFileName);

	if ( szPath[0] == '\0' )
		return 0;

	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_KINDOF(CDocTemplate, pTemplate);

		CDocTemplate::Confidence match;
		ASSERT(pOpenDocument == NULL);
		match = pTemplate->MatchDocType(szPath, pOpenDocument);
		if (match > bestMatch)
		{
			bestMatch = match;
			pBestTemplate = (CDocTemplFileSync *)pTemplate;
		}
		if (match == CDocTemplate::yesAlreadyOpen)
			break;      // stop here
	}

	if (pOpenDocument != NULL)
	{
		CDocTemplFileSync *pDocTempl = (CDocTemplFileSync*)pOpenDocument->GetDocTemplate();

		return pDocTempl->GetIconNo();
	}

	if (pBestTemplate == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_OPEN_DOC);
		return 0;
	}

	return pBestTemplate->GetIconNo();
}

CDocTemplFileSync* CDocManFileSync::FindTemplate( UINT nIDResource )
{
	POSITION pos = m_templateList.GetHeadPosition();
	while ( pos != NULL )
	{
		CDocTemplFileSync* pTemplate = (CDocTemplFileSync*)m_templateList.GetNext( pos );
		ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
		if ( pTemplate->GetResourceID() == nIDResource )
			return pTemplate;
	}
	ASSERT(FALSE);
	return NULL;
}

void CDocManFileSync::SetType( CDocTemplFileSync* pTemplate )
{
	if ( pTemplate == NULL )
		m_strType.Empty();
	else
	{
//		ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
		m_strType = pTemplate->GetTypeString();
	}
}
