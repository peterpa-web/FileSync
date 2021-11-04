// DocFileSync.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "DocTemplFileSync.h"
#include "DocDir.h"
#include "sys\stat.h"
#include "DocDirArchive.h"

#include "DocFileSync.h"


// CDocFileSync

IMPLEMENT_DYNAMIC(CDocFileSync, CDocument)

CDocFileSync::CDocFileSync()
{
//	TRACE0( "CDocFileSync::CDocFileSync()\n" );
//	TRACE2( "CDocFileSync::CDocFileSync() %x AutoDelete=%d\n", this, m_bAutoDelete );
	m_bReadOnly = FALSE;
	m_pParent = NULL;
	m_posParent = NULL;
#ifdef _DEBUG
	m_bLock = FALSE;	// is not in use
#endif
}

CDocFileSync::~CDocFileSync()
{
//	TRACE1( "CDocFileSync::~CDocFileSync() %x\n", this );
	ASSERT( !m_bLock );
}

//int CDocFileSync::GetIconNo()
//{
//	return ((CDocTemplFileSync*)GetDocTemplate())->GetIconNo();
//}

BOOL CDocFileSync::OnNewDocument()
{
	TRACE1( "CDocFileSync::OnNewDocument() %s\n", CString(GetRuntimeClass()->m_lpszClassName) );
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

void CDocFileSync::DeleteContents()
{
	TRACE2( "CDocFileSync::DeleteContents() %s %x\n", CString(GetRuntimeClass()->m_lpszClassName), this );
	ASSERT( !m_bLock );
	m_bReadOnly = FALSE;
	m_pParent = NULL;
	m_posParent = NULL;
}

BOOL CDocFileSync::ResetAll()
{
	m_bReadOnly = FALSE;
	SetModifiedFlag( FALSE );
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDocFileSync, CDocument)
END_MESSAGE_MAP()



// CDocFileSync diagnostics

#ifdef _DEBUG
CDocDir * CDocFileSync::GetParentDoc() const
{
	if (m_pParent != NULL) 
		m_pParent->AssertValid(); 
	return m_pParent; 
}

void CDocFileSync::AssertValid() const
{
	CDocument::AssertValid();
	if (m_pParent != NULL) 
		ASSERT_VALID(m_pParent);
}

void CDocFileSync::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG




// CDocFileSync commands

//void CDocFileSync::SetModifiedFlag( BOOL bModified /* = TRUE */ )
//{
//	CDocument::SetModifiedFlag( bModified );
//}

void CDocFileSync::SetPathName(LPCTSTR lpszPathName, BOOL /*bAddToMRU*/ )
{
//	TRACE1( "CDocFileSync::SetPathName() %s\n", CString(GetRuntimeClass()->m_lpszClassName) );
	m_strPathName = lpszPathName;
//	m_strPathNameView = m_strPathName;
}

//void CDocFileSync::SetPathNameView( const CString &strPath )
//{
//	m_strPathNameView = strPath;
//}

BOOL CDocFileSync::OnOpenDocument(LPCTSTR lpszPathName)
{
	ASSERT( FALSE );	// test only: identify calls without using progress
	return OnOpenDocument( lpszPathName, NULL, NULL );
}

void CDocFileSync::RefreshAttr()
{
	struct _stati64 fs;
	int result = _tstati64( GetPathName(), &fs );
	if ( result == 0 )
	{
		SetReadOnly( (fs.st_mode & S_IWRITE) == 0 );
	}
}

__int64 CDocFileSync::GetFileSize()
{
	struct _stati64 fs;
	int result = _tstati64( GetPathName(), &fs );
	if ( result == 0 )
	{
		return fs.st_size;
	}
	return 0;
}


BOOL CDocFileSync::IsReadOnlyFile()
{
	struct _stati64 fs;
	int result = _tstati64( GetPathName(), &fs );
	if ( result == 0 )
	{
		return( (fs.st_mode & S_IWRITE) == 0 );
	}
	return TRUE; // no real file
}

void CDocFileSync::SetReadOnly( BOOL b )
{
	m_bReadOnly = b;
	if ( m_pParent != NULL && m_posParent != NULL )
		m_pParent->GetDirEntry( m_posParent ).SetRO( b );
}

void CDocFileSync::SetReadOnlyFile( BOOL b )
{
	if ( m_pParent != NULL && m_posParent != NULL )
		m_pParent->MakeRW( m_posParent, !b );
	else {
		TRACE1("CDocFileSync::SetReadOnlyFile() %s\n", m_strPathName );
		int mode = !b ? _S_IREAD | _S_IWRITE : _S_IREAD;
		int result = _tchmod( m_strPathName, mode );
		ASSERT( result == 0 );
	}
}

void CDocFileSync::OnCloseDocument()
{
	TRACE0( "CDocFileSync::OnCloseDocument()\n" );

	CDocument::OnCloseDocument();
}

void CDocFileSync::OnChangedViewList()
{
	TRACE0( "CDocFileSync::OnChangedViewList()\n" );

	CDocument::OnChangedViewList();
}

BOOL CDocFileSync::CanCloseFrame(CFrameWnd* pFrame)
{
	TRACE0( "CDocFileSync::CanCloseFrame()\n" );

	return CDocument::CanCloseFrame(pFrame);
}

CString CDocFileSync::GetBasePathName()
{
	return GetBasePathName( GetPathName() );
}

CString CDocFileSync::GetBasePathName( CString strPath )
{
	int q = strPath.Find( '\\', 2 );
	if ( strPath.GetLength() > (q+1) && strPath.Right( 1 ) == '\\' )
		strPath = strPath.Left( strPath.GetLength()-1 );

	CString strDir;
	int p = strPath.ReverseFind( '\\' );
	if ( p > q )
		strDir = strPath.Left( p );
	else
		strDir = strPath.Left( p+1 );
	return strDir;
}

CString CDocFileSync::GetPathNameView() const
{
	ASSERT( m_pParent != (CDocDir*)0xfeeefeee && m_posParent != (POSITION)0xfeeefeee );
	if ( m_pParent != NULL && m_posParent != NULL )
	{
		CString str = m_pParent->GetPathNameView() + _T("\\");
		return str + m_pParent->GetDirEntry(m_posParent).GetName();
	}
	return GetPathName();
}

BOOL CDocFileSync::DoFileSave()
{
//	CString strOldView = m_strPathNameView; 050317
	BOOL b = CDocument::DoFileSave();
	if ( b )
	{
		if ( m_pParent != NULL && m_posParent != NULL )
			m_pParent->SetModifiedFlag( m_posParent, TRUE );
	}
//	m_strPathNameView = strOldView;
	return b;
}

BOOL CDocFileSync::DoSave(LPCTSTR lpszPathName, BOOL bReplace /* = TRUE */ )
{
	BOOL b = CDocument::DoSave( lpszPathName, bReplace );
	if ( b && (lpszPathName == NULL) ) {	// reset doc ref & mode
		m_pParent = NULL;
		m_posParent = NULL;
		m_bReadOnly = FALSE;
	}
	return b;
}

BOOL CDocFileSync::SaveModified( int nSide )
{
	if (!IsModified())
		return TRUE;        // ok to continue

	CString prompt = _T("Save changed data");
	if ( nSide == 0 )
		prompt += _T(" from\nleft side?");
	else if ( nSide == 1 )
		prompt += _T(" from\nright side?");
	else
		prompt += _T("?");
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL))
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;

	case IDNO:
		// If not saving changes, revert the document
		SetModifiedFlag( FALSE );
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}

CDocTemplFileSync* CDocFileSync::GetDocTemplate() const
{
	CDocTemplate *pTemplate = CDocument::GetDocTemplate();
	ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
	return (CDocTemplFileSync*)pTemplate;
}

BOOL CDocFileSync::IsNoTemp() const
{
	if ( m_pParent == NULL )
		return TRUE;

	return !m_pParent->IsKindOf( RUNTIME_CLASS(CDocDirArchive) );
}
