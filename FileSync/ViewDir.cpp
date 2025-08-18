#include "StdAfx.h"
#include "FileSync.h"
#include "MainFrm.h"
#include "DocDirNative.h"
#include "DocDirArchive.h"
#include "DualListBox.h"
#include "DocManFileSync.h"
#include "DocTemplFileSync.h"
#include "ColorsDlg.h"
#include "REBProgressDialog.h"
#include "UndoDir.h"
#include "DialogReplace.h"
#include "AssocDlg.h"
#include "TaskDirExp.h"
#include "TaskDirCompFiles.h"
#include "TaskDirDep.h"
#include "TaskIdleDel.h"
#include "TaskIdleInv.h"

#include "ViewDir.h"

static BOOL g_bInit = FALSE;


IMPLEMENT_DYNCREATE(CViewDir, CViewFileSync)

CViewDir::CViewDir(void) : m_tree( m_treeData )
{
	m_nTitleID = IDR_VIEWDIR;
	m_nMenueID = IDR_VIEWDIR;
	m_nToolbarID[0] = IDR_VIEWDIR;
	m_nToolbarID[1] = IDR_VIEWDIR_R;

	m_nWidthFileSize = 42;
	m_nCharOffs = 0;
	m_bUndoEnabled = TRUE;
	m_pTask = NULL;
	m_bInvalidate = FALSE;
	m_nSortType = ID_SORT_NAME;
}

CViewDir::~CViewDir(void)
{
	TRACE1( "CViewDir::~CViewDir() %x\n", this );
}

BEGIN_MESSAGE_MAP(CViewDir, CViewFileSync)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXT_DIFF, OnUpdateViewNextDiff)
	ON_COMMAND(ID_VIEW_NEXT_DIFF, OnViewNextDiff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREV_DIFF, OnUpdateViewPrevDiff)
	ON_COMMAND(ID_VIEW_PREV_DIFF, OnViewPrevDiff)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACESEL, OnUpdateEditReplacesel)
	ON_COMMAND(ID_EDIT_REPLACESEL, OnEditReplacesel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEL_LINES, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_DEL_LINES, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_LEFT, OnUpdateCmd)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_RIGHT, OnUpdateCmd)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPENDIRLEFT, OnUpdateCmd)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPENDIRRIGHT, OnUpdateCmd)
	ON_COMMAND(ID_FILE_OPEN_LEFT, OnFileOpenLeft)
	ON_COMMAND(ID_FILE_OPEN_RIGHT, OnFileOpenRight)
	ON_COMMAND(ID_FILE_OPENDIRLEFT, OnFileOpenDirLeft)
	ON_COMMAND(ID_FILE_OPENDIRRIGHT, OnFileOpenDirRight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_INCLUDESUBDIRS, OnUpdateViewIncludesubdirs)
	ON_COMMAND(ID_VIEW_INCLUDESUBDIRS, OnViewIncludesubdirs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_EXPLORER, OnUpdateViewExplorer)
	ON_COMMAND(ID_VIEW_EXPLORER, OnViewExplorer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILESYNC, OnUpdateViewFilesync)
	ON_COMMAND(ID_VIEW_FILESYNC, OnViewFilesync)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_PREV, OnUpdateEditPrev)
	ON_COMMAND(ID_EDIT_PREV, OnEditPrev)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NEXT, OnUpdateEditNext)
	ON_COMMAND(ID_EDIT_NEXT, OnEditNext)
	ON_UPDATE_COMMAND_UI(ID_TYPE_HEX, OnUpdateCmd)
	ON_UPDATE_COMMAND_UI(ID_TYPE_TEXT, OnUpdateCmd)
	ON_UPDATE_COMMAND_UI(ID_TYPE_XML, OnUpdateCmd)
	ON_COMMAND(ID_TYPE_HEX, OnTypeHex)
	ON_COMMAND(ID_TYPE_TEXT, OnTypeText)
	ON_COMMAND(ID_TYPE_XML, OnTypeXml)
	ON_UPDATE_COMMAND_UI(ID_TYPE_RW, OnUpdateTypeRW)
	ON_COMMAND(ID_TYPE_RW, OnTypeRW)
	ON_COMMAND(ID_VIEW_COLORS, OnViewColors)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnNMClick )
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnNMDblclk )
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_LIST, OnTvnItemexpanding )
	ON_UPDATE_COMMAND_UI(ID_TYPE_OPEN, OnUpdateTypeOpen)
	ON_COMMAND(ID_TYPE_OPEN, OnTypeOpen)
	ON_COMMAND(ID_DIR_COLLAPSE, OnDirCollapse)
	ON_UPDATE_COMMAND_UI(ID_DIR_FREE, OnUpdateDirFree)
	ON_COMMAND(ID_DIR_FREE, OnDirFree)
	ON_UPDATE_COMMAND_UI(ID_DIR_EXPANDPARTIAL, OnUpdateDirExpandpartial)
	ON_COMMAND(ID_DIR_EXPANDPARTIAL, OnDirExpandpartial)
	ON_COMMAND(ID_DIR_SETEQUAL, OnDirSetequal)
	ON_COMMAND(IDC_SEARCH_C, OnSearch)
	ON_COMMAND(ID_SEARCH_B, OnSearchB)
	ON_COMMAND(ID_SEARCH_F, OnSearchF)
	ON_UPDATE_COMMAND_UI(ID_DIR_SELECTDIFFS, &CViewDir::OnUpdateDirSelectdiffs)
	ON_COMMAND(ID_DIR_SELECTDIFFS, &CViewDir::OnDirSelectdiffs)
	ON_COMMAND(ID_VIEW_ASSOCIATIONS, OnViewAssociations)
	ON_COMMAND(ID_HELP_CONTEXT, &CViewDir::OnHelpContext)
//	ON_MESSAGE(WM_UPD_TREE, OnMsgUpdTree)
	ON_UPDATE_COMMAND_UI(ID_SORT_NAME, OnUpdateSortName)
	ON_UPDATE_COMMAND_UI(ID_SORT_MKS, OnUpdateSortMKS)
	ON_UPDATE_COMMAND_UI(ID_SORT_LEFT_SIZE, OnUpdateSortLeftSize)
	ON_UPDATE_COMMAND_UI(ID_SORT_LEFT_SIZE_DESC, OnUpdateSortLeftSizeDesc)
	ON_UPDATE_COMMAND_UI(ID_SORT_LEFT_TIME, OnUpdateSortLeftTime)
	ON_UPDATE_COMMAND_UI(ID_SORT_LEFT_TIME_DESC, OnUpdateSortLeftTimeDesc)
	ON_UPDATE_COMMAND_UI(ID_SORT_RIGHT_SIZE, OnUpdateSortRightSize)
	ON_UPDATE_COMMAND_UI(ID_SORT_RIGHT_SIZE_DESC, OnUpdateSortRightSizeDesc)
	ON_UPDATE_COMMAND_UI(ID_SORT_RIGHT_TIME, OnUpdateSortRightTime)
	ON_UPDATE_COMMAND_UI(ID_SORT_RIGHT_TIME_DESC, OnUpdateSortRightTimeDesc)
	ON_COMMAND(ID_SORT_NAME, OnSortName)
	ON_COMMAND(ID_SORT_MKS, OnSortMKS)
	ON_COMMAND(ID_SORT_LEFT_SIZE, OnSortLeftSize)
	ON_COMMAND(ID_SORT_LEFT_SIZE_DESC, OnSortLeftSizeDesc)
	ON_COMMAND(ID_SORT_LEFT_TIME, OnSortLeftTime)
	ON_COMMAND(ID_SORT_LEFT_TIME_DESC, OnSortLeftTimeDesc)
	ON_COMMAND(ID_SORT_RIGHT_SIZE, OnSortRightSize)
	ON_COMMAND(ID_SORT_RIGHT_SIZE_DESC, OnSortRightSizeDesc)
	ON_COMMAND(ID_SORT_RIGHT_TIME, OnSortRightTime)
	ON_COMMAND(ID_SORT_RIGHT_TIME_DESC, OnSortRightTimeDesc)
	ON_MESSAGE(WM_USER_STARTIDLE, OnUserStartIdle)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROJECT, OnUpdateViewProject)
	ON_COMMAND(ID_VIEW_PROJECT, OnViewProject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LINK, OnUpdateViewLink)
	ON_COMMAND(ID_VIEW_LINK, OnViewLink)
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WM_USER_ERR, OnUserErr)
END_MESSAGE_MAP()

void CViewDir::OnInitialUpdate2()
{
	VERIFY( m_buttonsLeft[0].Create(this, ID_FILE_OPENDIRLEFT, IDB_OPENDIR) );
	VERIFY( m_buttonsRight[0].Create(this, ID_FILE_OPENDIRRIGHT, IDB_OPENDIR) );
	VERIFY( m_buttonsLeft[1].Create(this, ID_FILE_OPEN_LEFT, IDB_OPEN) );
	VERIFY( m_buttonsRight[1].Create(this, ID_FILE_OPEN_RIGHT, IDB_OPEN) );
	VERIFY( m_buttonsLeft[2].Create(this, ID_FILE_SAVE_LEFT, IDB_SAVE, IDB_SAVE_DIS) );
	VERIFY( m_buttonsRight[2].Create(this, ID_FILE_SAVE_RIGHT, IDB_SAVE, IDB_SAVE_DIS) );

	CRect rect(5,50,200,250);
	m_tree.PrepareFont( &m_fontList );
	VERIFY( m_tree.Create( WS_VISIBLE | WS_VSCROLL | 
							TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
							rect, this, IDC_LIST ) );
	m_tree.SetImageList( &m_imagelist, TVSIL_NORMAL );
	m_tree.SetWndColor( m_crWndColor );
	m_tree.SetMarkDirColor( m_crMarkDir );
	m_tree.SetMarkDirColor2( m_crMark );
	m_tree.SetMarkLiteColor( m_crMarkLite );
	m_tree.SetTxtChangedColor( m_crTxtChanged );
	m_tree.SetMarkSingleColor( m_crMarkSingle );
	m_tree.AdjustVScroll();

	m_comboDirLeft.SetDirMode();
	m_comboDirRight.SetDirMode();

	m_threadBack.Start(m_hWnd);

	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("ViewDir") );
	if (hSecKey != NULL) {
		DWORD dwSort = (DWORD)m_nSortType;
		DWORD dwSize = 0;
		LONG lResult = RegQueryValueEx(hSecKey,  _T("sort"), NULL, NULL, (LPBYTE)&dwSort, &dwSize );
		if ( lResult == ERROR_SUCCESS )
			m_nSortType = dwSort;
		RegCloseKey(hSecKey);
	}
}

void CViewDir::MoveClient( LPCRECT lpRect )
{
	m_tree.MoveWindow( lpRect );
}

CTaskDirCompFiles *pTaskReComp = NULL;

void CViewDir::ResetSide()
{
	m_treeData.ResetAllDocRefs( m_tree.GetSide() );
}

// back from other view ...
BOOL CViewDir::ReactiveDocument(int nSide, CDocDir *pDoc, const CString& strBasePath)
{
	CDocDir *pCurrDoc = GetDoc(nSide);
	if ( pDoc == NULL ) // file view not called from dir view
	{
		CString strDir = pCurrDoc->GetPathName();
		CString strFile = strBasePath.Left(strDir.GetLength());
		if ( !strFile.IsEmpty() && strDir.CompareNoCase(strFile) != 0 ) {
			m_treeData.ResetAllDocRefs(nSide);
			TRACE0("CViewDir::ReactiveDocument failed\n");
			return FALSE;
		}

		SetDocument(pCurrDoc, nSide);
//		IdleRequest( UpdateIcons );
		return TRUE;
	}

	CDocDir *pd = pDoc;
	while ( pd != NULL )
	{
		if ( pCurrDoc == pd )
		{
			SetDocument(pCurrDoc, nSide);
			HTREEITEM hSel = m_tree.GetSelectedItem();
			if ( hSel != NULL ) {
				m_tree.GetItemData(hSel).UpdateMark();
				UpdateParentMark(hSel);
				// TODO: move behind CompareEnable(TRUE)
//				CTaskDirCompFiles *pTask = CTaskDirCompFiles::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemParentPos(hSel) );
//				m_threadBack.AddTask( pTask );
				if ( pTaskReComp != NULL )
					pTaskReComp->Delete();
				pTaskReComp = CTaskDirCompFiles::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemParentPos(hSel) );
				TRACE0( "CViewDir::ReactiveDocument pTaskReComp <==\n");
			}
//			IdleRequest( UpdateIcons );
			return TRUE;
		}
		pd = pd->GetParentDoc();
	}
	return FALSE;
}

void CViewDir::MarkSelEqual( BOOL b )
{
	// todo: don't markEqual if different lines were selected
	HTREEITEM hSel = m_tree.GetSelectedItem();
	if ( hSel != NULL && m_bSingleLine ) {
		if ( b )
			m_tree.GetItemData(hSel).SetMarkEqual();
		else
			m_tree.GetItemData(hSel).SetMarkDiff();
		UpdateParentMark(hSel);
	}
}

void CViewDir::SetDocument( CDocFileSync *pDocument, int nSide )
{
	TRACE2("CViewDir::SetDocument() %s, s=%d\n", CString(pDocument->GetRuntimeClass()->m_lpszClassName), nSide);
	if ( m_pProgressMan->GetVisible() )
		m_pProgressMan->SetVisible( FALSE );

	m_threadBack.CancelSide(nSide);

	CDocFileSync *pOldDoc = GetDoc(nSide);
	CDocTemplFileSync *pOldDocTemplate = NULL;
	if ( pOldDoc != NULL )
	{
		pOldDocTemplate = pOldDoc->GetDocTemplate();
	}
	if ( pDocument != pOldDoc )
	{
		m_treeData.ResetAllDocRefs( nSide );
		if ( pOldDocTemplate != NULL )
			pOldDocTemplate->RemoveDocument(pOldDoc);	// PP 050331
		delete pOldDoc;
	}

	m_pDoc[ nSide ] = pDocument;

	if ( !g_bInit )
	{
		g_bInit = TRUE;
		if ( !GetParentFrame()->RestoreWinPos() )
			GetParentFrame()->ShowWindow(SW_SHOW); // fallback if failed;
		InitDocs();
		return;		// done
	}

//	see CViewFileSync::SetDocument( pDocument, nSide );

	CWaitCursor wait;
	if ( !m_bUndoEnabled )
	{
		UpdateEditDir( nSide );
		if ( m_pTask == NULL ) {
			CompareView();
			SelectSide( left );	// 2010/02/18
		}
		return;
	}

	if ( m_pTask == NULL )
		m_pTask = new CUndoDir( this );
	if ( m_pTask == NULL )
		return;

	m_pTask->m_pOldDocTemplate[nSide] = pOldDocTemplate;
	m_pTask->m_pDocTemplate[nSide] = pDocument->GetDocTemplate();

	m_pTask->m_strOldPath[nSide] = m_strOldPath[nSide];
	m_pTask->m_strPath[nSide] = m_pDoc[nSide]->GetPathNameView();

	UpdateEditDir( nSide );
	if ( !m_bCompareEnabled )
		return;

	if ( !m_undoBuffer.AddTask( m_pTask ) )
	{
//		ASSERT( FALSE );
		delete m_pTask;
		CompareView();	// 20120508
	}
	m_pTask = NULL;
}

void CViewDir::InitDocs()
{
	CString strDirL = m_comboDirLeft.RestoreList();
	CString strDirR = m_comboDirRight.RestoreList();
	TRACE2( "CViewDir::InitDocs %s %s\n", strDirL, strDirR );
	m_undoBuffer.RemoveAll();
	m_threadBack.CancelAll();	// 20100525
	DeleteContents();
	InitDoc( strDirL, 0 );
	InitDoc( strDirR, 1 );
	CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, NULL, GetDoc(0), GetDoc(1), 1, m_nSortType, NULL );
	UpdateEditDir( 0 );
	UpdateEditDir( 1 );
	SelectSide( left );	// 2010/02/18
}

void CViewDir::InitDoc( const CString &strPath, int nSide )
{
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	CDocFileSync *pOldDoc = GetDoc(nSide);
	CDocTemplate *pOldDocTemplate = pOldDoc->GetDocTemplate();

	CDocFileSync *pDoc = pDM->CreateDocumentDir( strPath );
	if ( pDoc != NULL )
	{
		pOldDocTemplate->RemoveDocument(pOldDoc);
		delete pOldDoc;
		m_pDoc[ nSide ] = pDoc;
	}
/*
	CDocTemplFileSync *pTempl = pDM->FindTemplate( IDR_VIEWZIP );
	CDocument* pOpenDocument = NULL;
	CDocTemplate::Confidence match = pTempl->MatchDocType(strPath, pOpenDocument);
	if (match == CDocTemplate::yesAttemptNative) {
		m_threadBack.CancelSide(nSide);
//		CDocFileSync *pOldDoc = GetDoc(nSide);
//		CDocTemplate *pOldDocTemplate = NULL;
//		pOldDocTemplate = pOldDoc->GetDocTemplate();
		pOldDocTemplate->RemoveDocument(pOldDoc);
		delete pOldDoc;

		m_pDoc[ nSide ] = (CDocFileSync*)pTempl->CreateNewDocument();
	}
	else {
		pTempl = pDM->FindTemplate( IDR_VIEWISO );
		CDocTemplate::Confidence match = pTempl->MatchDocType(strPath, pOpenDocument);
		if (match == CDocTemplate::yesAttemptNative) {
			m_threadBack.CancelSide(nSide);
//			CDocFileSync *pOldDoc = GetDoc(nSide);
//			CDocTemplate *pOldDocTemplate = NULL;
//			pOldDocTemplate = pOldDoc->GetDocTemplate();
			pOldDocTemplate->RemoveDocument(pOldDoc);
			delete pOldDoc;

			m_pDoc[ nSide ] = (CDocFileSync*)pTempl->CreateNewDocument();
		}
	}
*/
	if ( GetDoc( nSide )->PreReScanAuto( nSide, strPath ) )
	{
		if ( GetDoc( nSide )->IsKindOf( RUNTIME_CLASS( CDocDirNative )) ) {
			changeNotify[nSide].MonitorDir( GetDoc( nSide )->GetPathName(), TRUE, 
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES);
		}
	}
	else
		Warn( nSide );
}

void CViewDir::CompareEnable( BOOL bEnable /* = TRUE */ )
{
	TRACE1( "CViewDir::CompareEnable %d\n", bEnable );
	m_bCompareEnabled = bEnable;

	if ( bEnable )
	{
//		changeNotify[0].Enable();
//		changeNotify[1].Enable();
		int nTreeSide = m_tree.GetSide();
		SelectSide( nTreeSide );
				// moved from ReactivateDocument
		if ( pTaskReComp != NULL ) {
			m_threadBack.AddTask( pTaskReComp );
			pTaskReComp = NULL;
		}
	}

	if ( !m_bCompareEnabled || m_pTask == NULL )
		return;

	if ( !m_undoBuffer.AddTask( m_pTask ) )
		delete m_pTask;
	m_pTask = NULL;
}

void CViewDir::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// TODO: Add your specialized code here and/or call the base class
}

void CViewDir::OnUpdateCmd(CCmdUI *pCmdUI)
{
	if ( m_pProgressMan->GetVisible() )
		pCmdUI->Enable( FALSE );
}

BOOL CViewDir::CheckUpdateCmd(CCmdUI *pCmdUI)
{
	if ( !m_pProgressMan->GetVisible() )
		return TRUE;

	pCmdUI->Enable( FALSE );
	return FALSE;
}

void CViewDir::ClearItemRecursive( HTREEITEM hItem, int nSide )
{
	CViewDirItem &d = m_tree.GetItemData( hItem );
	TRACE1( "CViewDir::ClearItemRecursive %s\n", d.GetName() );
	// check dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );
	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		ClearItemRecursive( hItem, nSide );
	}

	if ( d.IsPresent(nSide) )
	{
		d.GetDirEntry(nSide).SetDel();

		if ( d.GetDoc(nSide) != NULL )
			d.GetDoc(nSide)->ResetAll();
		d.UpdateMark();
	}

	if ( !d.IsPresent(1-nSide) )
	{
		m_tree.DeleteItem( hItem );
		return;
	}
	return;
}

BOOL CViewDir::RemoveItemRecursive( HTREEITEM hItem, int nSide )
{
	CViewDirItem &d = m_tree.GetItemData( hItem );
	// check dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );

	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		RemoveItemRecursive( hItem, nSide );
	}

	changeNotify[nSide].Signal();

	if ( !d.GetParentDoc(nSide)->RemoveFileDir( d.GetParentPos(nSide) ) )
		return FALSE;

//	if ( d.GetDoc(nSide) != NULL )
//		d.GetDoc(nSide)->ResetAll();

//	UpdateParentMark( hItem );
//	if ( !d.IsPresent(1-nSide) )
//	{
//		m_tree.DeleteItem( hItem );
//		return TRUE;
//	}
//	d.UpdateMark();
	return TRUE;
}

__int64 CViewDir::CopyItemRecursivePre( HTREEITEM hItem, int nSide )
{
	__int64 nSize = 0;

	// nSide is source side
	CViewDirItem &d = m_tree.GetItemData( hItem );

	if ( !d.IsPresent(nSide) )
	{
		return nSize;
	}
	m_pProgressMan->SetStaticText( 1, d.GetName() );
	nSize = d.GetFileSize(nSide);

	return nSize;
}

BOOL CViewDir::CopyItemRecursive( HTREEITEM hItem, int nSide, BOOL bPhysCopy )
{
	// nSide is source side
	CViewDirItem &d = m_tree.GetItemData( hItem );

	// check dependent items
	if ( bPhysCopy ) {
		HTREEITEM hChild = m_tree.GetChildItem( hItem );
		while ( hChild != NULL && !m_pProgressMan->GetUserAbortFlag() )
		{
			HTREEITEM hItem = hChild;
			hChild = m_tree.GetNextSiblingItem( hChild );
			if ( !CopyItemRecursive( hItem, nSide, bPhysCopy & !d.HasArcIcon() ) )
				return FALSE;
		}
	}
	if ( m_pProgressMan->GetUserAbortFlag() )
		return FALSE;

	if ( !d.IsPresent(nSide) )
	{
		return TRUE;
	}
	CString strName;
	TREEPOS posParent = d.GetItemParentPos();
	if ( posParent == NULL )
		strName = d.GetName();
	else
		strName = m_treeData.GetItemData(posParent).GetName() + _T("\\") + d.GetName();
	m_pProgressMan->SetStaticText( 1, strName );
	m_pProgressMan->SetProgressToBase();
	DOCPOS posDest = d.GetParentPos(1-nSide);
//	DOCPOS posDestOld = posDest;
	BOOL b = d.GetParentDoc(1-nSide)->CopyFile( posDest, 
											d.GetParentDoc(nSide), d.GetParentPos(nSide), TRUE,
											&CopyProgressRoutine, m_pProgressMan );
	if ( b )
		changeNotify[1-nSide].Signal();

	if ( !b )
	{
		if ( MessageBox( d.GetName() + _T("\n") + GetLastErrorText(), 
					NULL, MB_OKCANCEL | MB_ICONWARNING ) != IDOK )
		return FALSE;
	}
	return TRUE;
}

BOOL CViewDir::SaveModifItemRecursive( HTREEITEM hItem, int nSide, BOOL bRemoveExpanded )
{
	// check dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );
	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		if ( !SaveModifItemRecursive( hItem, nSide, bRemoveExpanded ) )
			return FALSE;
	}

	CViewDirItem &d = m_tree.GetItemData( hItem );
	if ( d.GetDoc(nSide) != NULL && !d.GetDoc(nSide)->GetPathName().IsEmpty() )
	{
		if ( !d.GetDoc(nSide)->SaveModified( nSide ) )
			return FALSE;
		if ( bRemoveExpanded ) {
			d.GetDoc(nSide)->RemoveTempRoot();
		}
	}
	return TRUE;
}

BOOL CViewDir::FindRW( HTREEITEM hItem, int nSide, BOOL &bRW )
{
	CViewDirItem &d = m_tree.GetItemData(hItem);
	if ( !d.IsPresent(nSide) )
		return FALSE;
	if ( !d.HasDirIcon() )
	{
		bRW = d.GetDirEntry(nSide).IsRO();	// toggle
		return TRUE;
	}

	// process dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );
	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		if ( FindRW( hItem, nSide, bRW ) )
			return TRUE;
	}
	return FALSE;
}

BOOL CViewDir::SetRWRecursive( HTREEITEM hItem, int nSide, BOOL bRW )
{
	CViewDirItem &d = m_tree.GetItemData(hItem);
	TRACE1( "CViewDir::SetRWRecursive %s\n", d.GetNameDebug() );
	if ( d.IsPresent(nSide) && 
		!d.HasDirIcon() )
	{
		CDirEntry &de = d.GetDirEntry(nSide);
		d.GetParentDoc( nSide )->MakeRW( de, bRW );
		de.SetRO( !bRW );
		return TRUE;
	}

	// process dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );
	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		if ( !SetRWRecursive( hItem, nSide, bRW ) )
			return FALSE;
	}
	return TRUE;
}

void CViewDir::DelZipContents( HTREEITEM hItem )
{
	CTaskIdleDel *pTaskIdleDel = NULL;
	// clear dependent items
	HTREEITEM hChild = m_tree.GetChildItem( hItem );
	while ( hChild != NULL )
	{
		HTREEITEM hItem = hChild;
		hChild = m_tree.GetNextSiblingItem( hChild );
		CViewDirItem &d = m_tree.GetItemData(hItem);
		TREEPOS pos = m_tree.GetItemPos( hItem );
		d.ClearItem(0);
		d.ClearItem(1);
		if ( pTaskIdleDel == NULL )
			pTaskIdleDel = CTaskIdleDel::New( &m_tree, pos );
		else
			pTaskIdleDel->AddPos( pos );
	}
	CViewDirItem &d = m_tree.GetItemData(hItem);
	if ( d.GetDoc(0) != NULL )
		d.GetDoc(0)->ResetAll();
	if ( d.GetDoc(1) != NULL )
		d.GetDoc(1)->ResetAll();

	if ( pTaskIdleDel != NULL )
		m_threadBack.AddIdleTask( pTaskIdleDel );
}

void CViewDir::DeleteContents()
{
	TRACE( "CViewDir::DeleteContents()\n" );
	if ( pTaskReComp != NULL ) {
		pTaskReComp->Delete();
		pTaskReComp = NULL;
	}
	m_threadBack.CancelAll();
	m_tree.DeleteAllItems();
	SetScrollPos( SB_HORZ, 0, FALSE );
	m_nCharOffs = 0;
	UpdateHScroll();
	m_tree.AdjustVScroll();
}

BOOL CViewDir::CompareView()
{
	TRACE( "CViewDir::CompareView()\n" );
	ASSERT( !m_pProgressMan->GetVisible() );

	if ( GetDocL()->IsKindOf( RUNTIME_CLASS( CDocDirNative )) ) {
		changeNotify[0].MonitorDir( GetDocL()->GetPathName(), TRUE, 
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES);
	}
	if ( GetDocR()->IsKindOf( RUNTIME_CLASS( CDocDirNative )) ) {
		changeNotify[1].MonitorDir( GetDocR()->GetPathName(), TRUE, 
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES);
	}
	m_bChanged[0] = TRUE;	// enables menu view refresh
//	GetParentFrame()->SetIndicator( ID_INDICATOR_LEFT, FALSE );
//	GetParentFrame()->SetIndicator( ID_INDICATOR_RIGHT, FALSE );

	OnRefresh();
	UpdateHScroll();
	return TRUE;
}

void CViewDir::ReCompareSide( int nSide, HTREEITEM hParentItem  )
{
	HTREEITEM hItem = ( hParentItem == NULL ? m_tree.GetRootItem() : m_tree.GetChildItem( hParentItem ) );
	CDocDir *pDoc;
	CDocDir *pDocL;
	CDocDir *pDocR;
	if ( hItem == NULL ) {
		if ( hParentItem == NULL )
			return;
		CViewDirItem &d = m_tree.GetItemData( hParentItem );
		pDoc = d.GetDoc( nSide );
		if ( pDoc == NULL )
			return;
		pDocL = d.GetDoc( left );
		pDocR = d.GetDoc( right );
	}
	else {
		CViewDirItem &d = m_tree.GetItemData( hItem );
		pDoc = d.GetParentDoc( nSide );
		pDocL = d.GetParentDoc( left );
		pDocR = d.GetParentDoc( right );
	}
	return;

//	BOOL bChanged = FALSE;	// 090402
//	if ( bChanged ) {
//		TRACE1( "CViewDir::ReCompareSide changed %s\n", pDoc->GetPathName() );
//		UpdateParentMark( hItem );
//		IdleRequest( UpdateIcons );
//	}
	// process dependent items
//	hItem = ( hParentItem == NULL ? m_tree.GetRootItem() : m_tree.GetChildItem( hParentItem ) );
//	while ( hItem != NULL )
//	{
//		if ( m_tree.GetItemData( hItem ).HasDirOrArcIcon() )			// IsDir()
//			ReCompareSide( nSide, hItem );
//		hItem = m_tree.GetNextSiblingItem( hItem );
//	}

//	m_tree.UpdCharsLeft(  GetFSCharCount(0) + 18 );
//	m_tree.UpdCharsRight( GetFSCharCount(1) + 18 );
//	UpdateHScroll();
//	GetParentFrame()->SetIndicator( nSide == 0 ? ID_INDICATOR_LEFT : ID_INDICATOR_RIGHT, FALSE );
}

BOOL CViewDir::Show( BOOL b )
{
	TRACE1( "CViewDir::Show %d\n", b );
	if ( b )
	{
		SetDlgCtrlID( AFX_IDW_PANE_FIRST );		// activate for CFrameWnd::RecalcLayout
		ShowWindow(SW_SHOW);
	}
	else
	{
		if ( m_pProgressMan->GetVisible() ) {
			TRACE0("CViewDir::Show disabling Close\n");
//			return FALSE;		// 20120330
		}

		// save documents ...
		if ( m_pDoc[0] != NULL )
		{
			if ( !m_pDoc[0]->SaveModified(0) )
				return FALSE;
		}
		if ( m_pDoc[1] != NULL )
		{
			if ( !m_pDoc[1]->SaveModified(1) )
				return FALSE;
		}
		HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("ViewDir") );
		if (hSecKey != NULL) {
			DWORD dwSort = (DWORD)m_nSortType;
			LONG lResult = RegSetValueEx(hSecKey, _T("sort"), NULL, REG_DWORD, (LPBYTE)&dwSort, sizeof(DWORD));
			RegCloseKey(hSecKey);
		}
		// don't DeleteContents()
		SetDlgCtrlID( m_nMenueID );		// deactivate for CFrameWnd::RecalcLayout
		ShowWindow(SW_HIDE);
	}
	return TRUE;
}


void CViewDir::OnUpdateViewNextDiff(CCmdUI *pCmdUI)
{
	int nSide = common;
	HTREEITEM hItem = m_tree.GetBottomItem();
#ifdef _DEBUG
	if ( hItem )
	{
		CString s = m_tree.GetItemData(hItem).GetName();
	}
#endif
	if ( hItem != NULL )
		nSide = m_tree.GetItemData(hItem).GetSideMarked();

	// skip non marked lines
	while ( hItem != NULL && !m_tree.GetItemData(hItem).IsMarkEqualorSingle() )
		hItem = m_tree.GetNextVisibleItem( hItem );
	// skip marked line with same side
	while ( hItem != NULL && m_tree.GetItemData(hItem).IsMarkEqualorSingle() ) {
		int nSideNew = m_tree.GetItemData(hItem).GetSideMarked();
		if (nSide == common)
			nSide = nSideNew;
		else if (nSide != nSideNew) {
			nSide = -1;		// found
			break;
		}
		hItem = m_tree.GetNextVisibleItem( hItem );
	}
	if ( hItem != NULL && (nSide == -1 || !m_tree.GetItemData(hItem).IsMarkEqualorSingle()) )
		pCmdUI->Enable();
	else
		pCmdUI->Enable(FALSE);
}

void CViewDir::OnViewNextDiff()
{
	HTREEITEM hItem = m_tree.GetBottomItem();

	// skip next non marked line
	while ( hItem != NULL && !m_tree.GetItemData(hItem).IsMarkEqualorSingle() )
		hItem = m_tree.GetNextVisibleItem( hItem );
	TRACE1("CViewDir::OnViewNextDiff next unmarked %s\n", m_tree.GetItemText(hItem) );
	int nSide = common;
	// skip marked line with same side
	while ( hItem != NULL && m_tree.GetItemData(hItem).IsMarkEqualorSingle() ) {
		int nSideNew = m_tree.GetItemData(hItem).GetSideMarked();
		if (nSide == common)
			nSide = nSideNew;
		else if (nSide != nSideNew) {
			nSide = -1;		// found
			break;
		}
		hItem = m_tree.GetNextVisibleItem( hItem );
	}
	if ( hItem != NULL && (nSide == -1 || !m_tree.GetItemData(hItem).IsMarkEqualorSingle()) )
	{
		TRACE1("CViewDir::OnViewNextDiff EnsureVisible %s\n", m_tree.GetItemText(hItem) );
		// m_tree.EnsureVisible( hItem );
		m_tree.SelectSetFirstVisible( hItem );
	}
}

void CViewDir::OnUpdateViewPrevDiff(CCmdUI *pCmdUI)
{
	int nSide = common;
	HTREEITEM hItem = m_tree.GetFirstVisibleItem();
	hItem = m_tree.GetPrevVisibleItem( hItem );
	while ( hItem != NULL )
	{
		if ( !m_tree.GetItemData(hItem).IsMarkEqualorSingle() ) {
			pCmdUI->Enable(TRUE);
			return;
		}
		int nSideNew = m_tree.GetItemData(hItem).GetSideMarked();
		if (nSide == common)
			nSide = nSideNew;
		else if (nSide != nSideNew) {
			pCmdUI->Enable(TRUE);
			return;
		}
		hItem = m_tree.GetPrevVisibleItem( hItem );
	}
	pCmdUI->Enable(FALSE);
}

void CViewDir::OnViewPrevDiff()
{
	int nSide = common;
	HTREEITEM hItem = m_tree.GetFirstVisibleItem();
	if ( hItem != NULL )
		nSide = m_tree.GetItemData(hItem).GetSideMarked();
	// skip prev non marked line
	hItem = m_tree.GetPrevVisibleItem( hItem );
	while ( hItem != NULL && !m_tree.GetItemData(hItem).IsMarkEqualorSingle() )
		hItem = m_tree.GetPrevVisibleItem( hItem );
	TRACE1("CViewDir::OnViewPrevDiff non marked %s\n", m_tree.GetItemText(hItem) );
	// skip prev last marked line
	while ( hItem != NULL && m_tree.GetItemData(hItem).IsMarkEqualorSingle() ) {
		int nSideNew = m_tree.GetItemData(hItem).GetSideMarked();
		if (nSide == common)
			nSide = nSideNew;
		else if (nSide != nSideNew) {
			nSide = -1;		// found
			break;
		}
		hItem = m_tree.GetPrevVisibleItem( hItem );
	}
	TRACE1("CViewDir::OnViewPrevDiff last marked %s\n", m_tree.GetItemText(hItem) );
	if ( hItem == NULL )
	{
		// find first marked line
		hItem = m_tree.GetRootItem();
		while ( hItem != NULL )
		{
			if ( !m_tree.GetItemData(hItem).IsMarkEqualorSingle() )
				break;
			hItem = m_tree.GetNextVisibleItem( hItem );
		}
	}
	else
	{
		// skip prev non marked lines
		while ( hItem != NULL )
		{
			if ( nSide == -1 || m_tree.GetItemData(hItem).IsMarkEqualorSingle() )
				break;
			hItem = m_tree.GetPrevVisibleItem( hItem );
		}
		TRACE1("CViewDir::OnViewPrevDiff prev non marked %s\n", m_tree.GetItemText(hItem) );
		if ( hItem == NULL )
		{
			hItem = m_tree.GetRootItem();
		}
	}
	if ( hItem != NULL )
	{
		TRACE1("CViewDir::OnViewPrevDiff EnsureVisible %s\n", m_tree.GetItemText(hItem) );
		m_tree.EnsureVisible( hItem );
	}
}

void CViewDir::UpdateHScroll()
{
	SCROLLINFO si;
	if ( !GetScrollInfo( SB_HORZ, &si ) )
	{
		si.cbSize = sizeof(si);
		si.nPos = 0;
		si.nTrackPos = 0;
	}
	si.nMin = 0;
	si.nMax = 0;
//	if ( m_pDoc[0] != NULL )
//		si.nMax = m_pDoc[0]->GetMaxLineLen();
//	if ( m_pDoc[1] != NULL && m_pDoc[1]->GetMaxLineLen() > si.nMax )
//		si.nMax = m_pDoc[1]->GetMaxLineLen();
	// TODO: adjust from view items instead
	if ( si.nPos > si.nMax )
		si.nPos = si.nMax;
	si.nPage = (m_tree.GetOffsLeft() - 18) / m_tree.GetCharWidth(); // visible common char count
	if ( si.nPage < 0 )
		si.nPage = 0;
	if ( si.nMax > (int)si.nPage )
	{
		if ( si.nPos > si.nMax )
			si.nPos = si.nMax;
	}
	else
	{
		si.nMax = 0;
		si.nPage = 0;
		si.nPos = 0;
	}
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
//	TRACE3( "CViewDir::UpdateHScroll nMax=%d nPage=%d nPos=%d\n", si.nMax, si.nPage, si.nPos );
	SetScrollInfo( SB_HORZ, &si );
}


void CViewDir::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if ( pScrollBar != NULL )
	{
		CViewFileSync::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	SCROLLINFO si;
	VERIFY( GetScrollInfo( SB_HORZ, &si ) );
    int nPosNew = si.nPos;
	TRACE1( "OnHScroll code=%d\n", nSBCode );

	switch( nSBCode )
	{
	case SB_RIGHT:	    // 7 VK_END
		nPosNew = si.nMax;
		break;
//	case SB_ENDSCROLL:	// 8 WM_KEYUP (the user released a key that sent a relevant virtual-key code)
	case SB_LINERIGHT:	// 1 VK_RIGHT or VK_DOWN
		if ( nPosNew < si.nMax )
			++nPosNew;
		break;
	case SB_LINELEFT:	    // 0 VK_LEFT or VK_UP
		if ( nPosNew > 0 )
			--nPosNew;
		break;
	case SB_PAGERIGHT:	// 3 VK_NEXT (the user clicked the channel below or to the right of the slider)
		if ( nPosNew < (si.nMax-(int)si.nPage) )
			nPosNew += si.nPage;
		else
			nPosNew = si.nMax;
		break;
	case SB_PAGELEFT:	    // 2 VK_PRIOR (the user clicked the channel above or to the left of the slider)
		if ( nPosNew > (int)si.nPage )
			nPosNew -= si.nPage;
		else
			nPosNew = 0;
		break;
	case SB_THUMBTRACK:	// 5 Slider movement (the user dragged the slider)
		nPosNew = nPos;
		break;
	case SB_LEFT:	    // 6 VK_HOME
		nPosNew = 0;
		break;
	}
	SetScrollPos( SB_HORZ, nPosNew );
	m_nCharOffs = nPosNew;
	m_tree.Invalidate();
	TRACE1( "OnHScroll %d\n", nPosNew );
}

void CViewDir::OnSize(UINT nType, int cx, int cy)
{
	CViewFileSync::OnSize(nType, cx, cy);

	UpdateHScroll();
}

HGLOBAL CViewDir::SelCopy()
{
	int nSide = s_nSide;
	CString strText;

	// compute size
	int nChar = 0;
	HTREEITEM hItem = m_tree.GetFirstSel();
	while ( hItem != NULL )
	{
		const CViewDirItem &d = m_tree.GetItemData(hItem);
		CString strFullPath = d.GetName();
		HTREEITEM hParentItem = m_tree.GetParentItem( hItem );
		while ( hParentItem != NULL )
		{
			const CViewDirItem &dp = m_tree.GetItemData(hParentItem);
			strFullPath = dp.GetName() + _T("\\") + strFullPath;
			hParentItem = m_tree.GetParentItem( hParentItem );
		}
		nChar += strFullPath.GetLength() + 2;
		strText += strFullPath + _T("\r\n");
		hItem = m_tree.GetNextSel( hItem );
	}

	if ( nChar < 1 )
		return NULL;	// nothing to copy

	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, (nChar + 1) * sizeof(TCHAR) );
	if (hg == NULL) 
		return NULL; 

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hg); 
	int nLen = strText.GetLength();
	memcpy(lptstrCopy, (LPCTSTR)strText, nLen * sizeof(TCHAR));
	lptstrCopy += nLen;
	*lptstrCopy = (TCHAR) 0;    // null character 
	GlobalUnlock(hg); 

	return hg;
}

void CViewDir::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_tree.IsAnySel() );
}

void CViewDir::OnEditCopy()
{
	if ( !OpenClipboard() )
		return;
	if ( !EmptyClipboard() )
		return;
	HGLOBAL hg = SelCopy();
	if ( hg != NULL )
#ifdef _UNICODE
		SetClipboardData(CF_UNICODETEXT, hg);
#else
		SetClipboardData(CF_TEXT, hg);
#endif
	CloseClipboard();
}



void CViewDir::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	OnUpdateEditReplacesel( pCmdUI );
}

void CViewDir::OnEditDelete()
{
	int nSide = s_nSide;

	CString str;
	str.Format( _T("Are you sure you want to delete %d files?"), m_tree.GetSelFiles( nSide ) );
	if ( MessageBox( str, 
					_T("FileSync Confirm Delete"), MB_YESNO | MB_ICONWARNING ) != IDYES )
		return;

	CWaitCursor waitCursor;
	HTREEITEM hItem = m_tree.GetFirstSel();
	while ( hItem != NULL )
	{
		HTREEITEM hItemNext = m_tree.GetNextSel(hItem);
		CViewDirItem &d = m_tree.GetItemData(hItem);
		if ( d.IsPresent(nSide) && 
			d.GetName() != _T("..") )
		{
			if ( d.HasArcIcon() && d.GetDoc(nSide) != NULL )
			{
				if ( SaveModifItemRecursive( hItem, nSide, TRUE ) ) {
					d.GetDirEntry(nSide).SetModif( FALSE );
				}
			}
			if ( !RemoveItemRecursive( hItem, nSide ) )
			{
				MessageBox( d.GetParentDoc(nSide)->GetFullPath(d.GetName()) + _T("\n") + 
								GetLastErrorText(CDocDir::GetLastError()), 
								NULL, MB_OK | MB_ICONWARNING );
				hItemNext = NULL; // cancel while
			}
		}
		hItem = hItemNext;
	}

	m_tree.Invalidate();
}


void CViewDir::OnUpdateEditReplacesel(CCmdUI *pCmdUI)
{
//	TRACE0( "OnUpdateEditReplacesel\n" );
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bEnable = m_tree.IsAnySel();
	if ( bEnable ) {
		if ( !m_tree.AllSelAreReady() ) {
			bEnable = FALSE;
		    HTREEITEM hItem = m_tree.GetFirstSel();
			while ( hItem != NULL && m_threadBack.IsReady() ) {
				CViewDirItem &d = m_tree.GetItemData(hItem);
				if ( d.IsDir() && d.IsMarkPending() && (s_nSide == 2 || d.IsPresent(s_nSide)) ) {
					TRACE1( "!!! OnUpdateEditReplacesel try to solve pending %s\n", d.GetNameDebug() );
					CTaskDirScan::CreateTasks( &m_threadBack, FALSE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), 
												d.GetDoc(0), d.GetDoc(1), 999, m_nSortType, NULL );
				}
				hItem = m_tree.GetNextSel(hItem);
			}
		}
	}
	pCmdUI->Enable( bEnable );
}

void CViewDir::OnEditReplacesel()
{
	int nSide = s_nSide;
	CDialogReplace dlg;
	if ( dlg.DoModal() != IDOK )
		return;

	CDocDir::ResetOverwriteNewer();
	m_pProgressMan->SetCaption( _T("FileSync Progress") );
	m_pProgressMan->ResetUserAbortFlag();
	m_pProgressMan->EnableCancel( TRUE );
	m_pProgressMan->SetAbortText( _T("Canceled ...") );
	m_pProgressMan->SetStaticText( 0, _T("Preparing") );
	m_pProgressMan->EnableProgress( TRUE );
	m_pProgressMan->SetProgress( 0 );
	m_pProgressMan->SetVisible( TRUE );
	m_comboDirLeft.EnableWindow( FALSE );
	m_comboDirRight.EnableWindow( FALSE );
	//m_tree.ShowWindow( SW_HIDE );
	m_tree.EnableClick( FALSE );

//	changeNotify[1-nSide].Enable( FALSE );
	__int64 nSize = 0;
	HTREEITEM hItem = m_tree.GetFirstSel();
	try {
		while ( hItem != NULL && !m_pProgressMan->GetUserAbortFlag() )
		{
			CViewDirItem &d = m_tree.GetItemData(hItem);
			if ( d.IsPresent(nSide) &&
				d.GetName() != _T("..") )
//					&&
//				d.GetName().Right(4).CompareNoCase( _T(".lnk") ) != 0 )
			{
				if ( d.HasArcIcon() )
				{
					SaveModifItemRecursive( hItem, nSide, TRUE );
					SaveModifItemRecursive( hItem, 1-nSide, FALSE );
				}
				nSize += CopyItemRecursivePre( hItem, nSide );
			}
			hItem = m_tree.GetNextSel( hItem );
		}
		//m_tree.ShowWindow( SW_SHOW );
		m_pProgressMan->SetRange( nSize );
		m_pProgressMan->SetStaticText( 0, _T("Copying") );

		hItem = m_tree.GetFirstSel();
		while ( hItem != NULL && !m_pProgressMan->GetUserAbortFlag() )
		{
			CViewDirItem &d = m_tree.GetItemData(hItem);
			if ( d.IsPresent(nSide) &&
				d.GetName() != _T("..") )
//					&&
//				d.GetName().Right(4).CompareNoCase( _T(".lnk") ) != 0 )
			{
				if ( d.HasArcIcon() ) {
					if ( !RemoveItemRecursive( hItem, 1-nSide ) )
						break;
				}
				if ( !CopyItemRecursive( hItem, nSide, !d.HasArcIcon() ) )
					break;
		//		if ( d.HasZipIcon() )
		//			DelZipContents( hItem );
			}
			hItem = m_tree.GetNextSel( hItem );
		}
	} catch ( CException *pe )
	{
		UINT nIDP = AFX_IDP_INTERNAL_FAILURE;   // generic message string
		pe->ReportError(MB_ICONSTOP, nIDP);
		pe->Delete();
	}
	m_pProgressMan->ResetUserAbortFlag();
	m_pProgressMan->SetVisible( FALSE );
	m_tree.EnableClick( TRUE );
	m_comboDirLeft.EnableWindow();
	m_comboDirRight.EnableWindow();

//	changeNotify[1-nSide].Enable();
//	m_tree.Invalidate();
}

void CViewDir::OnFileOpenRight()
{
	OnFileOpen(right);
}

void CViewDir::OnFileOpenLeft()
{
	OnFileOpen(left);
}

void CViewDir::OnFileOpen(int nSide)
{
	SelectSide( nSide );
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[nSide]->GetPathName();
	pDM->SetType( NULL );
	CString newName;
	if (!pDM->DoPromptFileName(newName, AFX_IDS_OPENFILE,
	        OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled
			
	m_threadBack.CancelSide(nSide);
//	m_treeData.ResetAllDocRefs( nSide );  20120405
	CompareEnable( FALSE );

	CDocFileSync *pd = 	(CDocFileSync*)AfxGetApp()->OpenDocumentFile(newName);
	if ( !pd->IsKindOf(RUNTIME_CLASS( CDocDirArchive )))
	{
		CDocTemplFileSync *pTemplate = pd->GetDocTemplate();
		SelectSide( 1-nSide );
		pDM->OpenDocumentFile( _T(""), pTemplate );
	}
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
	pView->UpdateEditDir(0);
	pView->UpdateEditDir(1);
	pView->CompareEnable( TRUE );
	pView->SelectSide( nSide );		// 2011/06/10
}

void CViewDir::OnFileOpenDirLeft()
{
	SelectSide( left );
	m_threadBack.CancelSide(left); // todo: erst nach erfolgreicher Dir Auswahl
	m_treeData.ResetAllDocRefs( left );
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[0]->GetPathName();
	CString str = pDM->OnDirOpen(m_pDoc[0]->GetPIDL());
	WarnReset( left );
}

void CViewDir::OnFileOpenDirRight()
{
	SelectSide( right );
	m_threadBack.CancelSide(right); // todo: erst nach erfolgreicher Dir Auswahl
	m_treeData.ResetAllDocRefs( right );
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	pDM->m_strInitialDir = m_pDoc[1]->GetPathName();
	CString str = pDM->OnDirOpen(m_pDoc[1]->GetPIDL());
	WarnReset( right );
}

void CViewDir::OnFileSave( Side s )
{
//	changeNotify[s].Enable( FALSE );
	if ( m_pDoc[s]->DoFileSave() )
		m_pDoc[s]->SetModifiedFlag( FALSE );
	Sleep(0);
	m_bChanged[s] = FALSE;
//	changeNotify[s].Enable();
	UpdateEditDir(s);
	UpdateModeButtons();
	m_undoBuffer.CleanModifiedFlag( s );
	changeNotify[s].Signal();
}


void CViewDir::OnUpdateViewIncludesubdirs(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	BOOL b = ( hItem != NULL );
	if ( b )
	{
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		b = ( d.HasDirOrArcIcon() && d.GetName() != _T("..") ); 
	}
	pCmdUI->Enable(b);
}

void CViewDir::OnViewIncludesubdirs()		// Expand All
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( !m_tree.IsSel( hItem ) )
	{
		CViewDirItem &d = m_tree.GetItemData( hItem );
#ifdef _DEBUG
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnViewIncludesubdirs !IsSel %s ###########\n", str);
#endif
		d.SetMarkDirty();
		UpdateParentMark( hItem );
		if ( d.HasArcIcon() ) {
			const CString &strName = d.GetName();
			if ( d.GetDoc(0) == NULL ) {
				d.SetDoc(0, CTaskDirDep::NewDocDirArcRoot( d.GetIcon(), strName, d.GetParentDoc(0), d.GetParentPos(0) ));
			}
			if ( d.GetDoc(1) == NULL ) {
				d.SetDoc(1, CTaskDirDep::NewDocDirArcRoot( d.GetIcon(), strName, d.GetParentDoc(1), d.GetParentPos(1) ));
			}
		}
        CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), d.GetDoc(0), d.GetDoc(1), 1999, m_nSortType, NULL );
		CTaskDirExp *pTaskDirExp = CTaskDirExp::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), 1999 );
		m_threadBack.AddTask( pTaskDirExp );
		m_tree.Expand( m_tree.GetItemPos( hItem ) );
		CTaskIdleInv *pTaskReady = CTaskIdleInv::New( &m_tree, m_tree.GetItemPos( hItem ), TRUE );
		m_threadBack.AddReadyTask( pTaskReady );
		return;
	}
	hItem = m_tree.GetFirstSel();
	while ( hItem != NULL )
	{
		UpdateParentMark( hItem );
		CViewDirItem &d = m_tree.GetItemData( hItem );
#ifdef _DEBUG
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnViewIncludesubdirs %s ###########\n", str);
#endif
//		d.SetExpandAll( TRUE );
		d.SetMarkDirty();
		if ( d.HasArcIcon() ) {
			const CString &strName = d.GetName();
			if ( d.GetDoc(0) == NULL ) {
				d.SetDoc(0, CTaskDirDep::NewDocDirArcRoot( d.GetIcon(), strName, d.GetParentDoc(0), d.GetParentPos(0) ));
			}
			if ( d.GetDoc(1) == NULL ) {
				d.SetDoc(1, CTaskDirDep::NewDocDirArcRoot( d.GetIcon(), strName, d.GetParentDoc(1), d.GetParentPos(1) ));
			}
		}
        CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), d.GetDoc(0), d.GetDoc(1), 1999, m_nSortType, NULL );
		CTaskDirExp *pTaskDirExp = CTaskDirExp::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), 1999 );
		m_threadBack.AddTask( pTaskDirExp );
		m_tree.Expand( m_tree.GetItemPos( hItem ) );
		hItem = m_tree.GetNextSel( hItem );
	}
	hItem = m_tree.GetFirstSel();
	if ( hItem != NULL ) {
//		CTaskIdleInv *pTaskReady = new CTaskIdleInv( m_tree, m_tree.GetItemPos( hItem ), TRUE );
		CTaskIdleInv *pTaskReady = CTaskIdleInv::New( &m_tree, m_tree.GetItemPos( hItem ), TRUE );
		m_threadBack.AddReadyTask( pTaskReady );
	}
}

void CViewDir::OnUpdateViewExplorer(CCmdUI *pCmdUI)
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	BOOL b = ( hItem != NULL );
	if ( b )
	{
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		b = ( d.IsDir() && d.IsPresent( m_tree.GetSide() ) ); 
	}
	pCmdUI->Enable(b);
}

void CViewDir::OnViewExplorer()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	TCHAR szCmd[MAX_PATH] = _T("explorer ");
	const CViewDirItem &d = m_tree.GetItemData( hItem );
	if ( d.GetName() == _T("..") )
		_tcsncat_s( szCmd, MAX_PATH, GetDoc( m_tree.GetSide() )->GetPathName() + _T("\\.."), MAX_PATH-10 );
	else
		_tcsncat_s( szCmd, MAX_PATH, d.GetDoc( m_tree.GetSide() )->GetPathName(), MAX_PATH-10 );

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process. 
	CreateProcess( NULL,  // No module name (use command line). 
		szCmd,			  // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi );            // Pointer to PROCESS_INFORMATION structure.
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
}

BOOL CViewDir::OnRefresh()
{
	if ( m_pProgressMan->GetVisible() )
		return TRUE;

	TRACE0("CViewDir::OnRefresh() =============================\n");
	m_threadBack.CancelAll();	// 20100525
	if ( !GetDocL()->SaveModified(0) )
		return FALSE;
	if ( !GetDocR()->SaveModified(1) )
		return FALSE;
	VERIFY( m_tree.DeleteAllItems() );
	VERIFY( GetDocL()->ResetAll() );
	VERIFY( GetDocR()->ResetAll() );
	BOOL bForce[2] = {TRUE, TRUE};
	CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, NULL, GetDocL(), GetDocR(), 1, m_nSortType, NULL, bForce );
	SelectSide( left );
	m_tree.AdjustVScroll();
	SaveHistory();		// 2011/03/14
	return TRUE;
}

void CViewDir::OnUpdateViewFilesync(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	pCmdUI->Enable( m_tree.IsAnySel() );
}

void CViewDir::OnViewFilesync()
{
	HTREEITEM hItem = m_tree.GetSelectedItem();
	OnTreeDblClk( hItem, common );
}

void CViewDir::OnTreeDblClk(HTREEITEM hItem, int nSide )
{
	OnTreeDblClk( hItem, nSide, NULL );
}

void CViewDir::OnTreeDblClk(HTREEITEM hItem, int nSide, CDocTemplFileSync* pTemplate )
{
	if ( hItem == NULL || m_pProgressMan->GetVisible() )
		return;

	TRACE0("CViewDir::OnTreeDblClk ====================== \n");
	CWaitCursor waitCursor;
	m_threadBack.CancelAll();
	CViewDirItem dL = m_tree.GetItemData( hItem );	// also default for single entry
	CViewDirItem dR = m_tree.GetItemData( hItem );
	CString strNameL = dL.GetName();
	CString strNameR = dR.GetName();
	CDocDir *pDocL = dL.GetParentDoc(0);
	CDocDir *pDocR = dR.GetParentDoc(1);
	DOCPOS posL = dL.GetParentPos(0);
	DOCPOS posR = dR.GetParentPos(1);
	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	m_bSingleLine = TRUE;

	if ( nSide == common )
	{
		HTREEITEM hItem2 = m_tree.GetFirstSel();	// find 2nd sel. item
		if ( hItem2 == hItem )
			hItem2 = m_tree.GetNextSel(hItem2);
		HTREEITEM hItem3 = m_tree.GetNextSel(hItem2);
		if ( hItem3 != NULL && hItem3 != hItem )
		{
			//Beep( 200, 100 );	// more than 2 selected items
			MessageBeep(MB_ICONWARNING);
			return;
		}
		if ( hItem2 != NULL )
		{
			m_bSingleLine = FALSE;
			if ( m_tree.GetItemData( hItem ).IsPresent(left) &&
				 m_tree.GetItemData( hItem2 ).IsPresent(right) )
			{
				dR = m_tree.GetItemData( hItem2 );
				strNameR = dR.GetName();
				pDocR = dR.GetParentDoc(1);
				posR = dR.GetParentPos(1);
			}
			else if ( m_tree.GetItemData( hItem2 ).IsPresent(left) &&
					m_tree.GetItemData( hItem ).IsPresent(right) )
			{
				dL = m_tree.GetItemData( hItem2 );
				strNameL = dL.GetName();
				pDocL = dL.GetParentDoc(0);
				posL = dL.GetParentPos(0);
			}
			else if ( m_tree.GetItemData( hItem ).IsPresent(left) &&
					m_tree.GetItemData( hItem2 ).IsPresent(left) )
			{
				dR = m_tree.GetItemData( hItem2 );
				strNameR = dR.GetName();
				pDocR = dR.GetParentDoc(0);
				posR = dR.GetParentPos(0);
			}
			else // 2x right
			{
				pDocL = dL.GetParentDoc(1);
				posL = dL.GetParentPos(1);
				dR = m_tree.GetItemData( hItem2 );
				strNameR = dR.GetName();
				pDocR = dR.GetParentDoc(1);
				posR = dR.GetParentPos(1);
			}
			if ( dL.HasDirIcon() != dR.HasDirIcon() )
			{
				//Beep( 200, 100 );	// different icon types
				MessageBeep(MB_ICONWARNING);
				return;
			}
		}

//		if ( !dL.IsPresent( left ) )
		if ( posL == NULL )
			nSide = right;
//		if ( !dR.IsPresent( right ) )
		if ( posR == NULL )
			nSide = left;
	}

//	if ( dL.HasArcIcon() && ( dL.GetParentDoc(nSide & 1)->IsKindOf( RUNTIME_CLASS( CDocDirArchive ) ) ) )
//	{
//		MessageBeep(MB_ICONWARNING);
//		return;
//	}

	if ( dL.HasDirIcon() )
	{
		m_treeData.ResetAllDocRefs(nSide);
		if ( nSide == common )
		{
			CString strFullPathL = pDocL->GetFullPathLnk( strNameL );
			CString strFullPathR = pDocR->GetFullPathLnk( strNameR );
			pDM->OpenDirs( strFullPathL, strFullPathR );
		}
		else if ( dL.IsPresent( nSide ) )
		{
			SelectSide( nSide );
			CString strFullPath = dL.GetParentDoc(nSide)->GetFullPathLnk( strNameL );
			pDM->OpenDocumentDir( strFullPath );
		}
	//	OnRefresh();	// 20121218 20130109
	}
	else
	{
		m_pProgressMan->SetCaption( _T("FileSync Progress") );
		m_pProgressMan->ResetUserAbortFlag();
		m_pProgressMan->EnableCancel( TRUE );
		m_pProgressMan->SetAbortText( _T("Canceled ...") );
		m_pProgressMan->EnableProgress( TRUE );
		m_pProgressMan->SetProgress( 0 );
		m_pProgressMan->SetVisible( TRUE );
		pDM->SetType( pTemplate );
		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		if ( nSide == common )
		{
			CompareEnable( FALSE );
			__int64 nSize = dL.GetFileSize( 0 ) + dR.GetFileSize( 1 );
			m_pProgressMan->SetRange( nSize );
			m_pProgressMan->SetStaticText( 0, _T("Opening") );
			try {
				SetSide( left );
				m_pProgressMan->SetStaticText( 1, _T("Left") );
				CDocFileSync *pd0 = pDocL->OpenDocumentFile( posL, pTemplate, &CopyProgressRoutine, m_pProgressMan );

				SetSide( right );
				m_pProgressMan->SetStaticText( 1, _T("Right") );
				m_pProgressMan->SetProgressToBase();
				CDocFileSync *pd1 = pDocR->OpenDocumentFile( posR, pTemplate, &CopyProgressRoutine, m_pProgressMan );
			
				CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
				if ( pd0 != NULL && pd1 != NULL ) {
					m_pProgressMan->SetStaticText( 0, _T("Comparing") );
					m_pProgressMan->SetStaticText( 1, _T("Both") );
					pView->UpdateEditDir( 0, pd0->IsNoTemp() );
					pView->UpdateEditDir( 1, pd1->IsNoTemp() );
					pView->CompareEnable( TRUE );
				}
				else {	// canceled
					if ( pView == this )
						CompareEnable( TRUE );
					else
						pView->OnViewDircomp();
				}
			} catch ( CFileException *pe ) {
				pe->Delete();
				CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
				if ( pView == this )
					CompareEnable( TRUE );
				else
					pView->OnViewDircomp();
			}
		}
		else if ( dL.IsPresent( nSide ) )
		{
			CompareEnable( FALSE );
			__int64 nSize = dL.GetFileSize( nSide );
			m_pProgressMan->SetRange( nSize );
			m_pProgressMan->SetStaticText( 0, _T("Opening") );
			try {
				SelectSide( nSide );
				m_pProgressMan->SetStaticText( 1, _T("") );
				CDocFileSync *pd = dL.GetParentDoc(nSide)->OpenDocumentFile( dL.GetParentPos(nSide), pTemplate, &CopyProgressRoutine, m_pProgressMan );
				if ( pd != NULL ) {
					if ( !dL.HasArcIcon() && dL.GetParentDoc(1-nSide) != NULL )		// && dL.IsPresent( 1-nSide )  removed 2011/03/17
					{
						pTemplate = pd->GetDocTemplate();
						SelectSide( 1-nSide );
						dL.GetParentDoc(s_nSide)->OpenDocumentFile( NULL, pTemplate, NULL, NULL );
					}
					m_pProgressMan->SetStaticText( 0, _T("Comparing") );
					CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
					pView->UpdateEditDir(0);
					pView->UpdateEditDir(1);
					pView->CompareEnable( TRUE );
					pView->SelectSide( nSide );		// 2011/06/10
				}
				else {	// canceled
					CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
					if ( pView == this )
						CompareEnable( TRUE );
					else
						pView->OnViewDircomp();
				}
			} catch ( CFileException *pe ) {
				pe->Delete();
				CViewFileSync *pView = (CViewFileSync *)pFrame->GetActiveView();
				if ( pView == this )
					CompareEnable( TRUE );
				else
					pView->OnViewDircomp();
			}
		}
		m_pProgressMan->ResetUserAbortFlag();
		m_pProgressMan->SetVisible( FALSE );
	}
}

CString CViewDir::GetLastErrorText(DWORD dwLastError)
{
	if ( dwLastError == 0 )
		dwLastError = GetLastError();
	CString strResult;
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM |
								FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,
                           dwLastError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), // Default language
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );
    if ( dwRet > 2 )
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
    }
    strResult.Format( TEXT("%s (Err=%d)"), lpszTemp, dwLastError );

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return strResult;
}

void CViewDir::OnTreeItemexpanding(LPNMTREEVIEW pNMTreeView)
{
	if ( m_tree.IsExpanding() )
		return;

	// handle manual event
	if ( pNMTreeView->action == TVE_EXPAND )	// ?? TVE_COLLAPSE
	{
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		CViewDirItem &d = m_tree.GetItemData(hItem);
		if ( d.HasArcIcon() ) {
			d.ResetMarkEqual();
			d.SetMarkPending();
		}
		if ( d.IsMarkPending() ) {
			d.SetMarkDirty();
			UpdateParentMark(hItem);
		}
#ifdef _DEBUG
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnTreeItemexpanding %s ###########\n", str);
#endif
		CreateSubItems( hItem, m_tree.GetItemPos(hItem), d );
	}
}

BOOL CViewDir::OnIdle(LONG lCount)
{
//	if ( lCount == 1 )
//		TRACE0( "CViewDir::OnIdle(1)\n" );

	m_buttonsLeft[2].EnableWindow( m_pDoc[left]->IsModified() );
	m_buttonsRight[2].EnableWindow( m_pDoc[right]->IsModified() );

	CString strMsg;
	if ( m_threadBack.IsReady() && m_threadBack.IsReadyIdle() ) {
		strMsg = _T("Ready");
		m_tree.SetBusyCursor( FALSE );
		if ( GetParentFrame()->UpdateMessageText( strMsg, 0 ) )
			return TRUE;
	} else {
		strMsg = m_threadBack.GetProgressString();
		int nProgress = m_threadBack.GetProgress();
		GetParentFrame()->UpdateMessageText( strMsg, nProgress );
		m_tree.SetBusyCursor();
	}

	if ( m_pProgressMan->GetVisible() )
		return FALSE;

	if ( m_threadBack.ProcessIdleStep() )
		return TRUE;

	if ( m_threadBack.IsReady() ) 
	{
		if ( m_tree.Optimize() )
			return TRUE;
	}

	if ( m_threadBack.IsReadyOpt() ) 
	{
//		TRACE0( "CViewDir::OnIdle IsReadyOpt\n" );
		BOOL b[2];
		b[0] = changeNotify[0].WhatIsChanged() != 0;
		b[1] = changeNotify[1].WhatIsChanged() != 0;
		if ( b[0] || b[1] ) {
			TRACE2( "CViewDir::OnIdle Refresh %d %d ########################\n", b[0], b[1] );
			CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, NULL, 
				GetDoc(0), GetDoc(1), 1, m_nSortType, NULL, b );
		}
	}

	Sleep( 300 );
	return TRUE;	// wait
}

void CViewDir::CreateSubItems( HTREEITEM hItem, TREEPOS posTree, CViewDirItem &d )
{
	CDocDir *pDocL = d.GetParentDoc(0);
	CDocDir *pDocR = d.GetParentDoc(1);
	ASSERT( pDocL != NULL );
	ASSERT( pDocR != NULL );
//	const CString &strName = d.GetName();
	BOOL bScanL = !d.IsValid(0);
	BOOL bScanR = !d.IsValid(1);
	d.SetMarkDirty();
	UpdateParentMark(hItem);

	CTaskDirDep::CreateSubDoc( d, 0 );
	CTaskDirDep::CreateSubDoc( d, 1 );

	CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, posTree, d.GetDoc(0), d.GetDoc(1), 1, m_nSortType, NULL );
}

void CViewDir::UpdateParentMark( HTREEITEM hItem )
{
	HTREEITEM hParentItem = m_tree.GetParentItem( hItem );
	if ( hParentItem == NULL )		// done
		return;

	CViewDirItem &dp = m_tree.GetItemData( hParentItem );
	if ( dp.IsMarkDirty() )
		return;

	dp.SetMarkAll();
//	TRACE1( "CViewDir::UpdateParentMark() %s\n", dp.GetName() );

	UpdateParentMark( hParentItem );	// do recursive
}

int CViewDir::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_strColorsSection = "Dir";
	CColorsDlg dlg;
	dlg.GetProfile( m_strColorsSection );
	m_crMarkDir = dlg.m_acr[0];
	m_crMarkLite = dlg.m_acr[1];
	m_crMark = dlg.m_acr[2];
	m_crTxtChanged = dlg.m_acr[3];
	m_crMarkSingle = dlg.m_acr[4];

	if (CViewFileSync::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY( m_menuContext.LoadMenu( IDR_MENU_TYPE ) );
	m_tree.SetMenuContext( &m_menuContext );
	VERIFY( m_menuContextDir.LoadMenu( IDR_MENU_DIR ) );
	m_tree.SetMenuContextDir( &m_menuContextDir );

	return 0;
}

void CViewDir::OnUpdateEditPrev(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	pCmdUI->Enable( m_undoBuffer.CanUndo() );
}

void CViewDir::OnEditPrev()
{
	TRACE0("CViewDir::OnEditPrev ====================== \n");
	m_undoBuffer.Undo();
}

void CViewDir::OnUpdateEditNext(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	pCmdUI->Enable( m_undoBuffer.CanRedo() );
}

void CViewDir::OnEditNext()
{
	TRACE0("CViewDir::OnEditNext ====================== \n");
	m_undoBuffer.Redo();
}

void CViewDir::OnTypeHex()
{
	OnType( IDR_VIEWHEX );
}

void CViewDir::OnTypeText()
{
	OnType( IDR_VIEWTEXT );
}

void CViewDir::OnTypeXml()
{
	OnType( IDR_VIEWXML );
}

void CViewDir::OnType( UINT nIDResource )
{
	HTREEITEM hItem = m_tree.GetItemCurr();
	const CViewDirItem &d = m_tree.GetItemData( hItem );
	if ( d.HasDirIcon() )
		return;

	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	CDocTemplFileSync *pTemplate = pDM->FindTemplate( nIDResource );

	OnTreeDblClk( hItem, m_tree.GetClickSide(), pTemplate );
}

void CViewDir::OnUpdateTypeRW(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;
	int nTreeSide = m_tree.GetClickSide();
	HTREEITEM hItem = m_tree.GetItemCurr();
	CViewDirItem &d = m_tree.GetItemData( hItem );
	if ( !d.IsSel() || (nTreeSide < 2 && nTreeSide != s_nSide) ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	OnUpdateEditReplacesel( pCmdUI );
	if ( !m_tree.AllSelAreReady() )
		return;

	int nSide = m_tree.GetClickSide();;
	CString strPerm;
	BOOL bRW;
	hItem = m_tree.GetFirstSel();
	if ( hItem == NULL )
		hItem = m_tree.GetItemCurr();
	while ( hItem != NULL )
	{
		HTREEITEM hItemNext = m_tree.GetNextSel(hItem);
		if ( FindRW( hItem, nSide, bRW ) )
			break;	// found
		hItem = hItemNext;
	}
	if ( hItem == NULL ) {
		pCmdUI->Enable( FALSE );
		return;
	}
	strPerm = bRW ? _T("Make R/W") : _T("Make R/O");
	pCmdUI->SetText( strPerm );
}

void CViewDir::OnTypeRW()
{
	TRACE0("CViewDir::OnTypeRW ====================== \n");
	int nSide = m_tree.GetClickSide();;
	CString strPerm;
	BOOL bRW;
	HTREEITEM hItem = m_tree.GetFirstSel();
	if ( hItem == NULL )
		hItem = m_tree.GetItemCurr();
	while ( hItem != NULL )
	{
		HTREEITEM hItemNext = m_tree.GetNextSel(hItem);
		if ( FindRW( hItem, nSide, bRW ) )
			break;	// found
		hItem = hItemNext;
	}
	if ( hItem == NULL ) {
		//Beep( 100, 100 );
		MessageBeep(MB_ICONWARNING);
		return;
	}

	strPerm = bRW ? _T("R/W?") : _T("R/O?");
	if ( MessageBox( _T("Are you sure you want to change the selected files to ") + strPerm, 
					_T("FileSync Confirm R/W"), MB_YESNO | MB_ICONWARNING ) != IDYES )
		return;

	CWaitCursor waitCursor;
	hItem = m_tree.GetFirstSel();
	while ( hItem != NULL )
	{
		HTREEITEM hItemNext = m_tree.GetNextSel(hItem);
		if ( SetRWRecursive( hItem, nSide, bRW ) )
			hItem = hItemNext;
		else
			hItemNext = NULL;	// cancel
	}

	m_tree.Invalidate();
}

void CViewDir::PostNcDestroy()
{
	m_menuContext.DestroyMenu();
	CViewFileSync::PostNcDestroy();
}

void CViewDir::OnViewColors()
{
	TRACE0("CViewDir::OnViewColors ====================== \n");
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
		m_tree.SetMarkDirColor( m_crMarkDir );
		m_tree.SetMarkDirColor2( m_crMark );
		m_tree.SetMarkLiteColor( m_crMarkLite );
		m_tree.SetTxtChangedColor( m_crTxtChanged );
		m_tree.SetMarkSingleColor( m_crMarkSingle );
		Invalidate();
	}
}

BOOL CViewDir::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ( wParam == IDC_LIST )
	{
		NMHDR *pNMH = (NMHDR*)lParam;
	}
	return CViewFileSync::OnNotify( wParam, lParam, pResult );
}

void CViewDir::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nTreeSide = m_tree.GetSide();
	if ( nTreeSide != s_nSide )
		SelectSide( nTreeSide );

	*pResult = 0;
}

void CViewDir::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hItem = m_tree.GetItemCurr();
	ASSERT((DWORD)hItem != 0xcdcdcdcd);
	OnTreeDblClk( hItem, m_tree.GetClickSide() );
	*pResult = 0;
}

void CViewDir::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	OnTreeItemexpanding( pNMTreeView );
	*pResult = 0;
}

void CViewDir::OnUpdateTypeOpen(CCmdUI *pCmdUI)
{
	// TODO: Fügen Sie hier Ihren Befehlsaktualisierungs-UI-Behandlungscode ein.
}

void CViewDir::OnTypeOpen()
{
	TRACE0("CViewDir::OnTypeOpen ====================== \n");
	HTREEITEM hItem = m_tree.GetItemCurr();
	CViewDirItem &d = m_tree.GetItemData( hItem );
	int nTreeSide = m_tree.GetClickSide();
	CDirEntry &de = d.GetDirEntry(nTreeSide);
	ShellExecute( GetParentFrame()->m_hWnd, _T("open"), de.GetName(), NULL, d.GetParentDoc( nTreeSide )->GetFullPathEx( TRUE ), SW_SHOW );
}

void CViewDir::OnDirCollapse()
{
	TRACE0("CViewDir::OnDirCollapse ====================== \n");
	if ( m_pProgressMan->GetVisible() )
		return;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( !m_tree.IsSel( hItem ) )
	{
		HTREEITEM hChildItem = m_tree.GetChildItem( hItem );
		while ( hChildItem != NULL )
		{
			m_tree.Collapse( hChildItem );
			hChildItem = m_tree.GetNextSiblingItem( hChildItem );
		}
		return;
	}

	hItem = m_tree.GetFirstSel();

	while ( hItem != NULL )
	{
#ifdef _DEBUG
		CViewDirItem &d = m_tree.GetItemData( hItem );
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnDirCollapse %s \n", str);
#endif
		HTREEITEM hChildItem = m_tree.GetChildItem( hItem );
		while ( hChildItem != NULL )
		{
			m_tree.Collapse( hChildItem );
			hChildItem = m_tree.GetNextSiblingItem( hChildItem );
		}
		hItem = m_tree.GetNextSel( hItem );
	}

}

void CViewDir::OnUpdateDirFree(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	BOOL b = ( hItem != NULL );
	if ( b )
	{
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		b = ( !m_tree.IsSel( hItem ) && d.GetName() != _T("..") ); 
	}
	pCmdUI->Enable(b);
}

void CViewDir::OnDirFree()
{
	if ( m_pProgressMan->GetVisible() )
		return;

	TRACE0("CViewDir::OnDirFree ====================== \n");
	m_threadBack.CancelAll();
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( !m_tree.IsSel( hItem ) )
	{
		m_tree.Collapse( hItem );
		m_tree.GetItemData( hItem ).SetMarkAll();
		m_tree.GetItemData( hItem ).SetMarkPending();
		m_tree.InvalidateItem( m_tree.GetItemPos( hItem ) );
		HTREEITEM hChildItem = m_tree.GetChildItem( hItem );
		while ( hChildItem != NULL )
		{
			HTREEITEM hItemFree = hChildItem;
			hChildItem = m_tree.GetNextSiblingItem( hChildItem );
			m_tree.Collapse( hItemFree );
			m_tree.FreeItemRecursive( m_tree.GetItemPos( hItemFree ) );
		}
	}

}

void CViewDir::OnUpdateDirExpandpartial(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	BOOL b = ( hItem != NULL );
	if ( b )
	{
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		b = ( d.HasDirOrArcIcon() && d.GetName() != _T("..") ); 
	}
	pCmdUI->Enable(b);
}

void CViewDir::OnDirExpandpartial()
{
//	CWaitCursor wait;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( !m_tree.IsSel( hItem ) )
	{
		CViewDirItem &d = m_tree.GetItemData( hItem );
#ifdef _DEBUG
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnDirExpandpartial !IsSel %s ###########\n", str);
#endif
		CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), d.GetDoc(0), d.GetDoc(1), -1999, m_nSortType, NULL );
		CTaskDirExp *pTaskDirExp = CTaskDirExp::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), -1999 );
		m_threadBack.AddTask( pTaskDirExp );
		CTaskIdleInv *pTaskReady = CTaskIdleInv::New( &m_tree, m_tree.GetItemPos( hItem ), TRUE );
		m_threadBack.AddReadyTask( pTaskReady );
		m_tree.Expand( m_tree.GetItemPos( hItem ) );
		TRACE0("CViewDir::OnDirExpandpartial ret\n");
		return;
	}
	hItem = m_tree.GetFirstSel();
	while ( hItem != NULL )
	{
		UpdateParentMark( hItem );
		CViewDirItem &d = m_tree.GetItemData( hItem );
#ifdef _DEBUG
		CString str = d.GetFullPath();	// debug only
		TRACE1("CViewDir::OnDirExpandpartial %s ###########\n", str);
#endif
		d.SetMarkDirty();
		CTaskDirScan::CreateTasks( &m_threadBack, TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), d.GetDoc(0), d.GetDoc(1), -1999, m_nSortType, NULL );
		CTaskDirExp *pTaskDirExp = CTaskDirExp::New( TRUE, &m_tree, &m_treeData, m_tree.GetItemPos( hItem ), -1999 );
		m_threadBack.AddTask( pTaskDirExp );
		m_tree.Expand( m_tree.GetItemPos( hItem ) );
		hItem = m_tree.GetNextSel( hItem );
	}
	hItem = m_tree.GetFirstSel();
	if ( hItem != NULL ) {
//		CTaskIdleInv *pTaskReady = new CTaskIdleInv( m_tree, m_tree.GetItemPos( hItem ), TRUE );
		CTaskIdleInv *pTaskReady = CTaskIdleInv::New( &m_tree, m_tree.GetItemPos( hItem ), TRUE );
		m_threadBack.AddReadyTask( pTaskReady );
	}
}

void CViewDir::OnDirSetequal()
{
	TRACE0("CViewDir::OnDirSetequal ====================== \n");
	HTREEITEM hItem = m_tree.GetItemCurr();
	CViewDirItem &d = m_tree.GetItemData( hItem );
	d.SetMarkEqual();
	UpdateParentMark( hItem );
	CRect rect;
	if ( m_tree.GetItemRect( hItem, &rect, FALSE ) )
		m_tree.InvalidateRect( &rect );
}

void CViewDir::OnSearch()
{
	OnSearchF();
}

void CViewDir::OnSearchB()
{
	TRACE0("CViewDir::OnSearchB ====================== \n");
	if ( m_pProgressMan->GetVisible() )
		return;
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CString strSearch = pFrame->GetSearchText().MakeLower();
	int nTreeSide = m_tree.GetSide();
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( hItem == NULL )
		return;
	hItem = m_tree.GetPrevVisibleItem( hItem );

	while ( hItem != NULL )
	{
		CString str = m_tree.GetItemText( hItem ).MakeLower();
//		TRACE1("B %s\n", str);
		CViewDirItem &d = m_tree.GetItemData( hItem );
		if ( (nTreeSide < 2 && strSearch == _T("rw") && !d.IsDir() && d.IsPresent( nTreeSide) && !d.GetDirEntry( nTreeSide ).IsRO() ) ||
			 str.Find( strSearch ) >= 0 ) {
			m_tree.EnsureVisible( hItem );
			m_tree.UnselectAll();
			m_tree.SelectItem( hItem );
			m_tree.Sel( hItem );
			m_tree.Invalidate();
			return;
		}
		hItem = m_tree.GetPrevVisibleItem( hItem );
	}
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

void CViewDir::OnSearchF()
{
	TRACE0("CViewDir::OnSearchF ====================== \n");
	if ( m_pProgressMan->GetVisible() )
		return;
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CString strSearch = pFrame->GetSearchText().MakeLower();
	//int nTreeSide = m_tree.GetSide();
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( hItem == NULL || !m_tree.IsSel( hItem ) ) {
		hItem = m_tree.GetRootItem();
	} else {
		hItem = m_tree.GetNextVisibleItem( hItem );
	}

	while ( hItem != NULL )
	{
		// CString str = m_tree.GetItemText( hItem ).MakeLower();
		// TRACE1("F %s\n", str);
		// CViewDirItem &d = m_tree.GetItemData( hItem );
		// if ( (nTreeSide < 2 && strSearch == _T("rw") && !d.IsDir() && d.IsPresent( nTreeSide) && !d.GetDirEntry( nTreeSide ).IsRO() ) ||
		// 	 str.Find( strSearch ) >= 0 ) {
		HTREEITEM hItemFound = SearchF( hItem, strSearch );
		if ( hItemFound != NULL ) {
			m_tree.EnsureVisible( hItemFound );
			m_tree.UnselectAll();
			m_tree.SelectItem( hItemFound );
			m_tree.Sel( hItemFound );
			m_tree.Invalidate();
			return;
		}
		HTREEITEM hItemNext = m_tree.GetNextSiblingItem( hItem );
		while ( hItemNext == NULL ) {
			hItem = m_tree.GetParentItem( hItem );
			if ( hItem == NULL )
				break;

			hItemNext = m_tree.GetNextSiblingItem( hItem );
		}
		hItem = hItemNext;
	}
	//Beep( 100, 100 );
	MessageBeep(MB_OK);
}

HTREEITEM CViewDir::SearchF( HTREEITEM hItem, const CString &strSearch )	// check incl. childs
{
	CString str = m_tree.GetItemText( hItem ).MakeLower();
	TRACE1("F %s\n", str);
	CViewDirItem &d = m_tree.GetItemData( hItem );
	int nTreeSide = m_tree.GetSide();
	if ( (nTreeSide < 2 && strSearch == _T("rw") && !d.IsDir() && d.IsPresent( nTreeSide) && !d.GetDirEntry( nTreeSide ).IsRO() ) ||
			str.Find( strSearch ) >= 0 ) {
		return hItem;
	}
	hItem = m_tree.GetChildItem( hItem );
	while ( hItem != NULL )
	{
		HTREEITEM hItemFound = SearchF( hItem, strSearch );
		if ( hItemFound != NULL )
			return hItemFound;

		hItem = m_tree.GetNextSiblingItem( hItem );
	}
	return NULL;
}

void CViewDir::OnUpdateDirSelectdiffs(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	HTREEITEM hItem = m_tree.GetSelectedItem();
	BOOL b = ( hItem != NULL );
	if ( b )
	{
		int nSide = m_tree.GetSide();
		if (nSide == common)
			nSide = s_nSide;
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		b = ( d.HasDirOrArcIcon() && d.IsPresent(nSide) &&
			( d.IsMarkDiff() || d.IsMarkSingle(nSide) ) ); 
	}
	pCmdUI->Enable(b);
}


void CViewDir::OnDirSelectdiffs()
{
	TRACE0("CViewDir::OnDirSelectdiffs ====================== \n");
	int nSide = m_tree.GetSide();
	SelectSide( nSide );

	HTREEITEM hItem = m_tree.GetSelectedItem();
	m_tree.UnselectAll();
	m_tree.SelDiffChilds(s_nSide, hItem);
	m_tree.Invalidate();
}

void CViewDir::ChangedSelCombo(int nSide, BOOL bReset)
{
	if ( bReset ) {
		m_threadBack.CancelSide(nSide);
		m_treeData.ResetAllDocRefs(nSide);	// ? 20120502
		WarnReset( nSide );					// 20120829
	}
	SelectSide(nSide);
}

void CViewDir::SelectSide(Side s)
{
	if ( m_tree.GetSide() != s ) {
		m_tree.SetSide((CDualTreeItem::Side)s);
		m_tree.Invalidate();
	}
	CViewFileSync::SelectSide(s);
}

void CViewDir::SelectSide2( Side s )
{
	CViewFileSync::SelectSide2(s);
	if ( m_tree.GetSide() != s ) {
		m_tree.SetSide((CDualTreeItem::Side)s);
		m_tree.Invalidate();
	}
}

BOOL CViewDir::GetMainIcon(HICON &hIcon, BOOL bBigIcon)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	int nPix = bBigIcon ? 32 : 16;
	HICON hIconNew = (HICON)LoadImage( hInst, MAKEINTRESOURCE(IDR_DIRCOMPFRAME), IMAGE_ICON, nPix, nPix, LR_SHARED);
	if ( hIconNew == hIcon )
		return FALSE;

	hIcon = hIconNew;
	return TRUE;
}

void CViewDir::OnViewAssociations()
{
	TRACE0("CViewDir::OnViewAssociations ====================== \n");
	CString str;
	HTREEITEM hItem = m_tree.GetSelectedItem();
	if ( hItem != NULL )
	{
		const CViewDirItem &d = m_tree.GetItemData( hItem );
		if ( !d.HasDirIcon() )
			str = d.GetName(); 
	}
	CAssocDlg dlg;
	dlg.m_strInitPath = str;
	dlg.DoModal();
}

void CViewDir::OnHelpContext()
{
	CFileSyncApp* pApp = (CFileSyncApp*)AfxGetApp();
	ShellExecute( GetParentFrame()->m_hWnd, _T("open"), _T("viewdir.html"), NULL, pApp->m_strHelpPath, SW_SHOW );
}

void CViewDir::OnUpdateSortName(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_NAME);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortMKS(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_MKS);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortLeftSize(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_LEFT_SIZE);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortLeftSizeDesc(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_LEFT_SIZE_DESC);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortLeftTime(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_LEFT_TIME);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortLeftTimeDesc(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_LEFT_TIME_DESC);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortRightSize(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_RIGHT_SIZE);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortRightSizeDesc(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_RIGHT_SIZE_DESC);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortRightTime(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_RIGHT_TIME);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnUpdateSortRightTimeDesc(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bOn = (m_nSortType == ID_SORT_RIGHT_TIME_DESC);
	pCmdUI->SetRadio( bOn );
	pCmdUI->Enable();
}
void CViewDir::OnSortName()
{
	m_nSortType = ID_SORT_NAME;
	OnRefresh();
}
void CViewDir::OnSortMKS()
{
	m_nSortType = ID_SORT_MKS;
	OnRefresh();
}
void CViewDir::OnSortLeftSize()
{
	m_nSortType = ID_SORT_LEFT_SIZE;
	OnRefresh();
}
void CViewDir::OnSortLeftSizeDesc()
{
	m_nSortType = ID_SORT_LEFT_SIZE_DESC;
	OnRefresh();
}
void CViewDir::OnSortLeftTime()
{
	m_nSortType = ID_SORT_LEFT_TIME;
	OnRefresh();
}
void CViewDir::OnSortLeftTimeDesc()
{
	m_nSortType = ID_SORT_LEFT_TIME_DESC;
	OnRefresh();
}
void CViewDir::OnSortRightSize()
{
	m_nSortType = ID_SORT_RIGHT_SIZE;
	OnRefresh();
}
void CViewDir::OnSortRightSizeDesc()
{
	m_nSortType = ID_SORT_RIGHT_SIZE_DESC;
	OnRefresh();
}
void CViewDir::OnSortRightTime()
{
	m_nSortType = ID_SORT_RIGHT_TIME;
	OnRefresh();
}
void CViewDir::OnSortRightTimeDesc()
{
	m_nSortType = ID_SORT_RIGHT_TIME_DESC;
	OnRefresh();
}

LRESULT CViewDir::OnUserStartIdle(UINT wParam, LONG lParam)
{
	TRACE0( "CViewDir::OnUserStartIdle\n" );
	return 0;
}

void CViewDir::OnUpdateViewProject(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( CDocDirNative::IsHidePJ()? 0 : 1 );
}

void CViewDir::OnViewProject()
{
	CDocDirNative::ToggleHidePJ();
	OnRefresh();
}

void CViewDir::OnUpdateViewLink(CCmdUI *pCmdUI)
{
	if ( !CheckUpdateCmd( pCmdUI ) )
		return;

	BOOL bEnable = FALSE;
	HTREEITEM hItem1 = m_tree.GetFirstSel();
	if ( hItem1 != NULL ) {
		HTREEITEM hItem2 = m_tree.GetNextSel( hItem1 );
		if ( hItem2 != NULL ) {
			HTREEITEM hItem3 = m_tree.GetNextSel( hItem2 );
			if ( hItem3 == NULL ) {
				HTREEITEM hItemPar1 = m_tree.GetParentItem( hItem1 );
				HTREEITEM hItemPar2 = m_tree.GetParentItem( hItem2 );
				if ( hItemPar1 == hItemPar2 ) {
					const CViewDirItem &d1 = m_tree.GetItemData( hItem1 );
					int nSide1 = d1.GetSide();
					const CViewDirItem &d2 = m_tree.GetItemData( hItem2 );
					int nSide2 = d2.GetSide();
					if ( (nSide1 & nSide2) == 0 && (nSide1 | nSide2) == 3 )
						bEnable = TRUE;
				}
			}
		}
	}
	pCmdUI->Enable( bEnable );
}

void CViewDir::OnViewLink()
{
	TRACE0("CViewDir::OnViewLink ====================== \n");
	m_threadBack.CancelAll();
	m_tree.Link();
}

BOOL CViewDir::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
    // We don't handle anything but scrolling.
    if ((nFlags & MK_CONTROL) != 0)
        return FALSE;

	m_tree.SetFocus();

    return FALSE;
}

LRESULT CViewDir::OnUserErr(UINT wParam, LONG lParam)
{
	CException* pe = (CException*)lParam;
	TCHAR szMsg[512] = _T("");
	pe->GetErrorMessage(szMsg, 512, NULL);
	CString s;
	if (wParam == 0)
		s = L"Left side:\n";
	else if (wParam == 1)
		s = L"Right side:\n";
	s += szMsg;
	pe->Delete();
	MessageBox(s, nullptr, MB_ICONEXCLAMATION);
	if (wParam == 0)
		m_comboDirLeft.SetWindowText(L"");
	else if (wParam == 1)
		m_comboDirRight.SetWindowText(L"");
	return 0;
}
