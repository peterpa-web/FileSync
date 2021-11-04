#include "stdafx.h"
#include "DocFileSync.h"
#include "ViewFileSync.h"
#include "MainFrm.h"
#include "Shlwapi.h"

#include "DocTemplFileSync.h"

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDocTemplFileSync construction/destruction

CDocTemplFileSync::CDocTemplFileSync(UINT nIDResource, CRuntimeClass* pDocClass,
	CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
		: CDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
	ASSERT(m_docList.IsEmpty());

	m_hMenuShared = NULL;
	m_hAccelTable = NULL;
//	m_nUntitledCount = 0;   // start at 1
	m_pView = NULL;
	m_bDefault = FALSE;
	m_strSection = _T("HEX");	// default only

	// load resources in constructor if not statically allocated
//	if (!CDocManager::bStaticInit)
		LoadTemplate();
}

void CDocTemplFileSync::LoadTemplate()
{
	CDocTemplate::LoadTemplate();

	if (m_nIDResource != 0 && m_hMenuShared == NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(
			MAKEINTRESOURCE(m_nIDResource), RT_MENU);
		m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(m_nIDResource));
		m_hAccelTable =
			::LoadAccelerators(hInst, MAKEINTRESOURCE(m_nIDResource));
	}

#ifdef _DEBUG
	// warnings about missing components (don't bother with accelerators)
	if (m_hMenuShared == NULL)
		TRACE1("Warning: no shared menu for document template #%d.\n",
			m_nIDResource);
#endif //_DEBUG
}

CDocTemplFileSync::~CDocTemplFileSync()
{
#ifdef _DEBUG
	if (!m_docList.IsEmpty()) {
		TRACE1("Warning: destroying CDocTemplFileSync with %d documents alive.\n",
			m_docList.GetCount());
	}
#endif
	// delete shared components
	if (m_hMenuShared != NULL)
		::DestroyMenu(m_hMenuShared);
	if (m_hAccelTable != NULL)
		::FreeResource((HGLOBAL)m_hAccelTable);
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplFileSync attributes

POSITION CDocTemplFileSync::GetFirstDocPosition() const
{
	return m_docList.GetHeadPosition();
}

CDocument* CDocTemplFileSync::GetNextDoc(POSITION& rPos) const
{
	return (CDocument*)m_docList.GetNext(rPos);
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplFileSync document management (a list of currently open documents)

void CDocTemplFileSync::AddDocument(CDocument* pDoc)
{
	ASSERT_VALID(pDoc);
	TRACE( "CDocTemplFileSync::AddDocument() %x\n", pDoc );

	CDocTemplate::AddDocument(pDoc);
	ASSERT(m_docList.Find(pDoc, NULL) == NULL); // must not be in list
	m_docList.AddTail(pDoc);
}


void CDocTemplFileSync::RemoveDocument(CDocument* pDoc)
{
	ASSERT_VALID(pDoc);

	TRACE( "CDocTemplFileSync::RemoveDocument() %x\n", pDoc );
	CDocTemplate::RemoveDocument(pDoc);
	m_docList.RemoveAt(m_docList.Find(pDoc));
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplFileSync commands

CDocTemplate::Confidence CDocTemplFileSync::MatchDocType(LPCTSTR lpszPathName,
														CDocument*& rpDocMatch)
{
//	TRACE0( "CDocTemplFileSync::MatchDocType()\n" );

//	Confidence cf = CDocTemplate::MatchDocType( lpszPathName, rpDocMatch );
//	if ( cf == yesAlreadyOpen )
//		return cf;
	
	LPCTSTR lpszDot = ::PathFindExtension(lpszPathName);
	if (lpszDot == NULL )
		return noAttempt;

	for ( int n = 0; n < m_astrExt.GetSize(); ++n )
	{
		if ( lstrcmpi( lpszDot, m_astrExt[n] ) == 0 )
			return yesAttemptNative; // extension matches, looks like ours
	}

	if ( m_bDefault )
		return yesAttemptForeign; // try ours

	return noAttempt;
}

CDocFileSync* CDocTemplFileSync::ReOpenDocumentFile(CDocFileSync* pDocument, LPCTSTR lpszPathName, 
					BOOL bMakeVisible, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
	// if lpszPathName == NULL => create new file of this type
{
	TRACE0( "CDocTemplFileSync::ReOpenDocumentFile()\n" );
	if (lpszPathName == NULL || *lpszPathName == '\0' )
	{
		// create a new document
		SetDefaultTitle(pDocument);

		// avoid creating temporary compound file when starting up invisible
		if (!bMakeVisible)
			pDocument->m_bEmbedded = TRUE;

		if (!pDocument->OnNewDocument())
		{
			// user has been alerted to what failed in OnNewDocument
			TRACE0("CDocument::OnNewDocument returned FALSE.\n");
			return NULL;
		}
	}
	else
	{
		// open an existing document
		CWaitCursor wait;
		pDocument->SetModifiedFlag(FALSE);  // not dirty for open

		if (!pDocument->OnOpenDocument(lpszPathName, lpProgressRoutine, lpData))
		{
			// user has been alerted to what failed in OnOpenDocument
			TRACE0("CDocument::OnOpenDocument returned FALSE.\n");
			SetDefaultTitle(pDocument);

			if (!pDocument->OnNewDocument())
			{
				TRACE0("Error: OnNewDocument failed after trying to open a document - trying to continue.\n");
				// assume we can continue
			}
			return NULL;        // open failed
		}
		pDocument->SetPathName(lpszPathName);
	}
	return pDocument;
}

CDocument* CDocTemplFileSync::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible /* = TRUE */)
{
	return OpenDocumentFile( lpszPathName, TRUE, bMakeVisible, NULL, NULL );
}

CDocument* CDocTemplFileSync::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible )
{
	return OpenDocumentFile( lpszPathName, bAddToMRU, bMakeVisible, NULL, NULL );
}

CDocument* CDocTemplFileSync::OpenDocumentFile( LPCTSTR lpszPathName, BOOL bAddToMRU,
	BOOL bMakeVisible, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
	// if lpszPathName == NULL => create new file of this type
{
	// Begin PP
	TRACE1( "CDocTemplFileSync::OpenDocumentFile(%s)\n", lpszPathName );
	CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	if (pFrame != NULL)
	{
		ASSERT_KINDOF(CMainFrame, pFrame);
		ASSERT_VALID(pFrame);
		CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
		ASSERT ( pView != NULL );

		CDocFileSync* pDocument = NULL;
		pDocument = pView->GetCurrDoc();
//		if ( pView->IsKindOf( m_pViewClass ) )
		if ( strcmp( pView->GetRuntimeClass()->m_lpszClassName, m_pViewClass->m_lpszClassName ) == 0 )
		{			// reuse view
			int nSide = pView->GetSide();
			if (!pDocument->SaveModified( nSide ))
				return NULL;        // leave the original one
			if ( !pDocument->IsKindOf( m_pDocClass ) )
			{
				//delete pDocument;
				//pDocument = (CDocFileSync*)m_pDocClass->CreateObject();	// PP 050331
				//AddDocument( pDocument );
				pDocument = (CDocFileSync*)CreateNewDocument();
			}
			if ( !pView->Show( FALSE ) )		// 20100217
				return NULL;
			pView->DeleteContents();			// 20120312
			if ( lpProgressRoutine == NULL ) {
				lpProgressRoutine = pView->GetProgressRoutine();
				lpData = pView->GetProgressMan();
				pView->StartProgress( pDocument->GetFileSize() );
			}
			if ( ReOpenDocumentFile( pDocument, lpszPathName, bMakeVisible, lpProgressRoutine, lpData ) == NULL ) {
				pView->GetProgressMan()->SetVisible( FALSE );
				return NULL;
			}
			pFrame->SetFocus();
			pFrame->SetActiveView( pView );		// 20100128
		}
		else	// other view type
		{
			if ( !pView->Show( FALSE ) )
				return NULL;

			pView = m_pView;		// pFrame->FindView( m_pViewClass );
			if ( pView == NULL )
			{		// create view
				CCreateContext context;
				context.m_pCurrentFrame = pFrame;
				context.m_pCurrentDoc = NULL;
				context.m_pNewViewClass = m_pViewClass;
				context.m_pNewDocTemplate = this;
				m_pView = (CViewFileSync*) pFrame->CreateView( &context );
//				if (!pFrame->LoadFrame(m_nIDResource,
//						0,		// create view only
//						NULL, &context))
//				{
//					TRACE(traceAppMsg, 0, "Warning: CDocTemplFileSync couldn't create a view.\n");
//					return NULL;
//				}
//				m_pView = (CViewFileSync*)pFrame->GetActiveView();
				pView = m_pView;
			}

			ASSERT_KINDOF(CViewFileSync, pView);
			ASSERT_VALID(pView);
			InitialUpdateFrame(pFrame, pDocument, bMakeVisible);
			pFrame->SetActiveView( pView );
			pDocument = pView->GetCurrDoc();
			if ( lpProgressRoutine == NULL ) {
				lpProgressRoutine = pView->GetProgressRoutine();
				lpData = pView->GetProgressMan();
				pView->StartProgress( pDocument->GetFileSize() );
			}
			if ( ReOpenDocumentFile( pDocument, lpszPathName, bMakeVisible, lpProgressRoutine, lpData ) == NULL ) {
				pView->GetProgressMan()->SetVisible( FALSE );
				return NULL;
			}
			pView->SetCurrDocument( pDocument );		// incl. compareView()
			return pDocument;
		}	// other view type

		ASSERT( pDocument != NULL );
//		InitialUpdateFrame(pFrame, pDocument, bMakeVisible);
		pView->SetCurrDocument( pDocument );		// incl. compareView()
		pView->Show(TRUE);	// 20100217
		return pDocument;
	}
	// no frame is present
	// End PP

	CDocument* pDocument = CreateNewDocument();
	if (pDocument == NULL)
	{
		TRACE0("CDocTemplate::CreateNewDocument returned NULL.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return NULL;
	}
	ASSERT_VALID(pDocument);

	BOOL bAutoDelete = pDocument->m_bAutoDelete;
	pDocument->m_bAutoDelete = FALSE;   // don't destroy if something goes wrong
	pFrame = (CMainFrame*)CreateNewFrame(pDocument, NULL);
	ASSERT_KINDOF(CMainFrame, pFrame);
	pDocument->m_bAutoDelete = bAutoDelete;
	if (pFrame == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		delete pDocument;       // explicit delete on error
		return NULL;
	}
	ASSERT_VALID(pFrame);

	if (lpszPathName == NULL)
	{
		// create a new document - with default document name
		SetDefaultTitle(pDocument);

		// avoid creating temporary compound file when starting up invisible
		if (!bMakeVisible)
			pDocument->m_bEmbedded = TRUE;

		if (!pDocument->OnNewDocument())
		{
			// user has be alerted to what failed in OnNewDocument
			TRACE0("CDocument::OnNewDocument returned FALSE.\n");
			pFrame->DestroyWindow();
			return NULL;
		}

		// it worked, now bump untitled count
//		m_nUntitledCount++;

		CWinApp* pApp = AfxGetApp();	// prepare hidden frame wnd; RestoreWin should follow
		ASSERT(pApp!=NULL);
		if (pApp->m_pMainWnd == NULL)
		{
			pApp->m_pMainWnd = pFrame;
			pApp->m_nCmdShow = SW_HIDE;
		}
		pFrame->InitialUpdateFrame(pDocument, bMakeVisible);	// 20120511
	}
	else
	{
		// open an existing document
		CWaitCursor wait;
		CDocFileSync *pDoc = (CDocFileSync*)pDocument;
		if ( lpProgressRoutine == NULL ) {
			CViewFileSync *pView = (CViewFileSync *)(pFrame->GetNewClient());
			lpProgressRoutine = pView->GetProgressRoutine();
			lpData = pView->GetProgressMan();
			pView->StartProgress( pDoc->GetFileSize() );
		}
		pFrame->InitialUpdateFrame(pDocument, bMakeVisible);	// 20120511
		if (!pDoc->OnOpenDocument( lpszPathName, lpProgressRoutine, lpData ))
		{
			// user has be alerted to what failed in OnOpenDocument
			TRACE0("CDocument::OnOpenDocument returned FALSE.\n");
			pFrame->DestroyWindow();
			return NULL;
		}
		pDocument->SetPathName(lpszPathName);
		CWinApp* pApp = AfxGetApp();
		ASSERT(pApp!=NULL);
		if (pApp->m_pMainWnd == NULL)
		{
			pApp->m_pMainWnd = pFrame;
			pApp->m_nCmdShow = -1; // set to default after first time
		}
	}

	// InitialUpdateFrame(pFrame, pDocument, bMakeVisible);
//	pFrame->InitialUpdateFrame(pDocument, bMakeVisible);

	CViewFileSync *pView = (CViewFileSync*)pFrame->GetActiveView();
	ASSERT( pView != NULL );
	pFrame->SetActiveView( pView );		// set menue etc.
//	pFrame->AddView( pView );
	m_pView = pView;
	pView->SetDocument( (CDocFileSync*)pDocument, pView->GetSide() );	// incl. InitDocs and CompareView
	pView->Show(TRUE);	// 20100218
	return pDocument;
}

void CDocTemplFileSync::SetDefaultTitle(CDocument* pDocument)
{
	CString strDocName;
	if (!GetDocString(strDocName, CDocTemplate::docName) ||
		strDocName.IsEmpty())
	{
		// use generic 'untitled'
		VERIFY(strDocName.LoadString(AFX_IDS_UNTITLED));
	}
	pDocument->SetTitle(strDocName);
}

CFrameWnd* CDocTemplFileSync::CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther)
{
	TRACE1( "CDocTemplFileSync::CreateNewFrame() %s\n", CString(m_pViewClass->m_lpszClassName) );
	if (pDoc != NULL)
		ASSERT_VALID(pDoc);
	// create a frame wired to the specified document

	ASSERT(m_nIDResource != 0); // must have a resource ID to load from
	CCreateContext context;
	context.m_pCurrentFrame = pOther;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = m_pViewClass;
	context.m_pNewDocTemplate = this;

	if (m_pFrameClass == NULL)
	{
		TRACE0("Error: you must override CDocTemplate::CreateNewFrame.\n");
		ASSERT(FALSE);
		return NULL;
	}
	CFrameWnd* pFrame = (CFrameWnd*)m_pFrameClass->CreateObject();
	// CFrameWnd* pFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd;	// PP
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			m_pFrameClass->m_lpszClassName);
		return NULL;
	}
	ASSERT_KINDOF(CFrameWnd, pFrame);

	if (context.m_pNewViewClass == NULL)
	{
		TRACE0("Warning: creating frame with no default view.\n");
	}
	// create new from resource
	// PP: CMainFrm implements LoadFrame to remove old view and create new view
	if (!pFrame->LoadFrame(m_nIDResource,
//			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,   // default frame styles
//			0,		// PP only view ...
			WS_OVERLAPPEDWINDOW,   // PP: default frame styles
			NULL, &context))
	{
		TRACE0("Warning: CDocTemplate couldn't create a frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	// it worked !
	return pFrame;
}

void CDocTemplFileSync::GetProfile( LPCTSTR pszSection, LPCSTR pszDefault )
{
	m_strSection = pszSection;
	m_astrExt.RemoveAll();
	HKEY hSecKey = AfxGetApp()->GetSectionKey( pszSection );
	if ( hSecKey == NULL )
		return;

	int n;
	for ( n = 0; TRUE; ++n )
	{
		TCHAR szEntry[10];
		_itot_s( n+1, szEntry, 10, 10 );
		CString strValue;
		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx(hSecKey, szEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lResult != ERROR_SUCCESS)
			break;

		ASSERT(dwType == REG_SZ);
		lResult = RegQueryValueEx(hSecKey, szEntry, NULL, &dwType,
			(LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
		strValue.ReleaseBuffer();

		strValue = CString( '.' ) + strValue;
		int i;
		for ( i = n-1; i >= 0; --i )
		{
			if ( m_astrExt[i] < strValue )
				break;
		}
		//m_astrExt.Add( CString( '.' ) + strValue );
		m_astrExt.InsertAt( i+1, strValue );
	}
	RegCloseKey( hSecKey );
	if ( n != 0 )
		return;

	LPCSTR psz = pszDefault;
	for ( n = 0; *psz != '\0'; ++n )
	{
		CString strValue(psz);
		strValue = CString( '.' ) + strValue;
		int i;
		for ( i = n-1; i >= 0; --i )
		{
			if ( m_astrExt[i] < strValue )
				break;
		}
		m_astrExt.InsertAt( i+1, strValue );
		psz += strValue.GetLength();
	}
}

void CDocTemplFileSync::WriteProfile() const
{
	ASSERT( !m_strSection.IsEmpty() );
	AfxGetApp()->WriteProfileString( m_strSection, NULL, NULL );	// delete section

	HKEY hSecKey = AfxGetApp()->GetSectionKey( m_strSection );
	ASSERT( hSecKey != NULL );
	if (hSecKey == NULL)
		return;
	for ( int n = 0; n < m_astrExt.GetSize(); ++n )
	{
		TCHAR szEntry[10];
		_itot_s( n+1, szEntry, 10, 10 );
		CString strValue = m_astrExt[n].Mid(1);
		LPCTSTR pszValue = strValue;
		LONG lResult = RegSetValueEx(hSecKey, szEntry, NULL, REG_SZ,
			(LPBYTE)pszValue, (lstrlen(pszValue)+1)*sizeof(TCHAR));
	}
	RegCloseKey(hSecKey);
}

CString CDocTemplFileSync::GetTypeString() const
{
	CString str = m_strSection;
	str.MakeLower();
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// CDocTemplFileSync diagnostics

#ifdef _DEBUG
void CDocTemplFileSync::Dump(CDumpContext& dc) const
{
	CDocTemplate::Dump(dc);

	dc << "m_hMenuShared = " << m_hMenuShared;
	dc << "\nm_hAccelTable = " << m_hAccelTable;
//	dc << "\nm_nUntitledCount = " << m_nUntitledCount;
	dc << "\nwith " << m_docList.GetCount();
	dc << " open documents";
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		dc << "\nwith document " << (void*)pDoc;
	}

	dc << "\n";
}

void CDocTemplFileSync::AssertValid() const
{
	CDocTemplate::AssertValid();

	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument* pDoc = GetNextDoc(pos);
		ASSERT_VALID(pDoc);
	}
}
#endif //_DEBUG

IMPLEMENT_DYNAMIC(CDocTemplFileSync, CDocTemplate)

/////////////////////////////////////////////////////////////////////////////
