#include "StdAfx.h"
#include "ViewDir.h"
#include "DocDir.h"
#include "DocTemplFileSync.h"
#include "DocManFileSync.h"
#include "MainFrm.h"

#include "UndoDir.h"

CUndoDir::CUndoDir(CViewDir *pView)
{
	m_pView = pView;
	m_pOldDocTemplate[0] = NULL;
	m_pOldDocTemplate[1] = NULL;
	m_pDocTemplate[0] = NULL;
	m_pDocTemplate[1] = NULL;
}

CUndoDir::~CUndoDir(void)
{
}

BOOL CUndoDir::DoFirst(void)
{
	TRACE0( "CUndoDir::DoFirst\n" );

	if ( m_strPath[0] == m_strPath[1] )
	{
		m_strPath[1] = "";
	}

	if ( m_strOldPath[0] == m_strPath[0] &&
	     m_strOldPath[1] == m_strPath[1] )
	{
//		ASSERT( FALSE );
		return FALSE;
	}
//	ASSERT( m_strOldPath[0] != m_strPath[0] || m_strOldPath[1] != m_strPath[1] );

	return TRUE;
}

BOOL CUndoDir::Do(void)
{
	TRACE0( "CUndoDir::Do\n" );

	m_pView->m_bUndoEnabled = FALSE;
	BOOL bRet = DoSide( 0, m_strPath[0], m_pDocTemplate[0] );

	if ( bRet )
		bRet = DoSide( 1, m_strPath[1], m_pDocTemplate[1] );
	m_pView->m_bUndoEnabled = TRUE;

	if ( !bRet )
		return FALSE;

	bRet = m_pView->CompareView();
	if ( !bRet )
		return FALSE;

	m_pView->SelectSide( CViewFileSync::left );	// 2010/02/18
	return TRUE;
}

BOOL CUndoDir::DoSide( int nSide, const CString &strPath, CDocTemplFileSync *pDocTemplate )
{
	if ( pDocTemplate == NULL )
		return TRUE;

	// see CDocTemplFileSync::OpenDocumentFile
	CDocDir* pDocOld = m_pView->GetDoc( nSide );
	if (!pDocOld->SaveModified( nSide ))
		return FALSE;        // leave the original one

	CDocDir *pDocument = NULL;
	if ( pDocOld->GetDocTemplate() == pDocTemplate )
		pDocument = pDocOld;
	else
	{
		pDocument = (CDocDir*)pDocTemplate->CreateNewDocument();
	}
	// see CDocTemplFileSync::ReOpenDocumentFile
	CWaitCursor wait;
	pDocument->SetModifiedFlag(FALSE);  // not dirty for open
	m_pView->ChangedSelCombo( nSide, TRUE );	// 20100818

	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	TCHAR szBasePath[_MAX_PATH];
	_tcscpy_s( szBasePath, _MAX_PATH, strPath );

	CDocTemplFileSync* pTemplate = pDM->FindDirTemplate( szBasePath );
	ASSERT( pTemplate == NULL || pTemplate == pDocTemplate );

	if (!pDocument->OnOpenDocument(strPath, NULL, NULL ))			// TODO: progress
	{
		if ( pDocOld != pDocument )
		{
			pDocTemplate->RemoveDocument(pDocument);	// PP 050331
			delete pDocument;
		}
		return FALSE;
	}

	pDocument->SetPathName( strPath );
	if ( pTemplate != NULL )
		pDocument->SetBasePath( szBasePath );
	m_pView->SetDocument( pDocument, nSide );
//	pDocument->ScanPath( strPath, TRUE );			to me moved ....
	pDocument->Invalidate();
	return TRUE;
}

BOOL CUndoDir::Undo(void)
{
	TRACE0( "CUndoDir::Undo\n" );

	m_pView->m_bUndoEnabled = FALSE;
	BOOL bRet = DoSide( 0, m_strOldPath[0], m_pOldDocTemplate[0] );

	if ( bRet )
		bRet = DoSide( 1, m_strOldPath[1], m_pOldDocTemplate[1] );
	m_pView->m_bUndoEnabled = TRUE;

	if ( !bRet )
		return FALSE;

//	bRet = m_pView->CompareView();
//	if ( !bRet )
//		return FALSE;

	m_pView->SelectSide( CViewFileSync::left );	// 2010/02/18
	return TRUE;
}

INT_PTR CUndoDir::GetSize()
{
	return 0;
}

void CUndoDir::SetModifiedFlag(int nChan, BOOL bFlag)
{
}



