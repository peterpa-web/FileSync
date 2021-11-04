#include "StdAfx.h"
//#include "UnZipClass.h"
//#include "ZipClass.h"
//#include <sys/stat.h>
#include "ViewFileSync.h"

#include "DocDirZipRoot.h"

#ifdef _DEBUG

void AssertX(BOOL b);
void AssertX(void *p) { AssertX(p != NULL); }
void AssertX1(BOOL b, LPCSTR szText);

#undef ASSERT
#define ASSERT(f) ::AssertX(f)
#define ASSERT1(f,t) ::AssertX1(f,t)
#else
#define AssertX(f)      ((void)0)
#define AssertX1(f,t)   ((void)0)
#define ASSERT1(f,t)   ((void)0)
#endif
#define ZIP_ARCHIVE_MFC
#include "ZipArchive.h"

IMPLEMENT_DYNCREATE(CDocDirZipRoot, CDocDirZipSub)

BOOL CDirEntry::IsZip() const
{
	if ( m_pd == NULL )
		return FALSE;
	return m_pd->IsKindOf(RUNTIME_CLASS( CDocDirZipRoot ));
}

// #########################

class CDocDirZipCallback : public CZipActionCallback
{
public:
	CDocDirZipCallback( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, __int64 nSize );
	void SetTotal( ZIP_SIZE_TYPE uTotalToDo );
	bool Callback( ZIP_SIZE_TYPE uProgress );

private:
	LPPROGRESS_ROUTINE m_lpProgressRoutine;
	LPVOID m_pProgressMan;
	LARGE_INTEGER m_nTotal;
	LARGE_INTEGER m_nTransferred;
	LARGE_INTEGER m_nStreamSize;
	LARGE_INTEGER m_nStreamTransf;
	DWORD m_dwStreamNumber;
};

CDocDirZipCallback::CDocDirZipCallback( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, __int64 nSize )
{
	m_lpProgressRoutine = lpProgressRoutine;
	m_pProgressMan = lpData;
	m_nTotal.QuadPart = nSize;
	m_nTransferred.QuadPart = 0;
	m_nStreamSize.QuadPart = 0;
	m_nStreamTransf.QuadPart = 0;
	m_dwStreamNumber = 0;
}

void CDocDirZipCallback::SetTotal( ZIP_SIZE_TYPE uTotalToDo )
{
	TRACE1( "CDocDirZipCallback::SetTotal %d\n", uTotalToDo );
	++m_dwStreamNumber;
	if ( m_lpProgressRoutine != NULL ) {
		DWORD dwReason = CALLBACK_STREAM_SWITCH;
		m_nStreamSize.QuadPart = uTotalToDo;
		m_nStreamTransf.QuadPart = 0;
		DWORD rc = (*m_lpProgressRoutine)( m_nTotal, m_nTransferred, m_nStreamSize, m_nStreamTransf, m_dwStreamNumber, dwReason, NULL, NULL, m_pProgressMan );
	}
}

bool CDocDirZipCallback::Callback( ZIP_SIZE_TYPE uProgress )
{
	TRACE1( "CDocDirZipCallback::Callback %d\n", uProgress );
	if ( m_lpProgressRoutine != NULL ) {
		DWORD dwReason = CALLBACK_CHUNK_FINISHED;
		m_nTransferred.QuadPart += uProgress;
		m_nStreamTransf.QuadPart += uProgress;
		DWORD rc = (*m_lpProgressRoutine)( m_nTotal, m_nTransferred, m_nStreamSize, m_nStreamTransf, m_dwStreamNumber, dwReason, NULL, NULL, m_pProgressMan );
		return ( rc != PROGRESS_CANCEL );
	}
	return true;
}

// #########################

CStorage<CDocDirZipRoot> CDocDirZipRoot::s_storeDocs( 10, _T("CDocDirZipRoot") );

CDocDirZipRoot::CDocDirZipRoot(void)
{
//	m_bExtracted = FALSE;
	m_pArcRoot = this;
}

//CDocDirZipRoot::CDocDirZipRoot(const CString &strName, CDocDir *pParent, DOCPOS posParent)
//{
//	m_strName = strName;
//	m_pParent = pParent;
//	m_posParent = posParent;
//	VERIFY( strName == pParent->GetDirEntry(posParent).GetName() );
//	m_bExtracted = FALSE;
//	m_pZipRoot = this;
//}

CDocDirZipRoot* CDocDirZipRoot::New( const CString &strName, CDocDir *pParent, DOCPOS posParent ) 
{
    ASSERT( posParent != NULL );

	POSITION posDoc = s_storeDocs.New();
    CDocDirZipRoot *pDoc = s_storeDocs.GetPtrAt(posDoc);
	pDoc->m_posStorage = posDoc;
	TRACE2( "CDocDirZipRoot::New( %s ) %x\n", strName, pDoc );
	pDoc->Init( strName, pParent, posParent );
    return pDoc;
}

CDocDirZipRoot::~CDocDirZipRoot(void)
{
	TRACE2( "CDocDirZipRoot::~CDocDirZipRoot() %x %s\n", this, m_strName );
	ASSERT( !IsModified() );
	DeleteContents();	// remove subitems & temp dir
}

void CDocDirZipRoot::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeDocs.DeleteAt( pos );
    }
    else
        delete this;
}

void CDocDirZipRoot::DeleteContents()
{
	TRACE1( "CDocDirZipRoot::DeleteContents() %s\n", m_strName );
	CDocDirZipSub::DeleteContents();
    m_arcRoot.DeleteContents();
//	ResetAll();
}

BOOL CDocDirZipRoot::ResetAll()
{
	if ( !RemoveTempRoot() )
		return FALSE;

	if ( CDocDirZipSub::ResetAll() ) {
//		CDocDir::DeleteContents();
		return TRUE;
	}
	return FALSE;
}

BOOL CDocDirZipRoot::CheckPath()
{
	int result = _taccess( m_arcRoot.GetBasePath(), 0 );
	return ( result == 0 );
}

BOOL CDocDirZipRoot::OnOpenDocument( LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE2( "CDocDirZipRoot::OnOpenDocument( %s ) %s\n", lpszPathName, 
		CString(GetRuntimeClass()->m_lpszClassName) );
#ifdef _DEBUG
	if (IsModified()) {
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");
	}
#endif

	DeleteContents();

	CDocDir::SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}


BOOL CDocDirZipRoot::PreReScanAuto( int nSide, const CString &strBasePath )
{
	return PreReScanAutoBase( nSide, m_arcRoot.GetBasePath(), _S_IFREG );
}

BOOL CDocDirZipRoot::ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
//	if ( !SaveModified() )			TODO
//		return FALSE;
	SET_DOC_LOCK(TRUE);
	CString strBasePath = GetPathName();
	if ( strBasePath.IsEmpty() )
	{
		if ( GetParentDoc() == NULL ) {
			SET_DOC_LOCK(FALSE);
			return FALSE;
		}
		strBasePath = GetParentDoc()->GetFullPathEx( m_posParent, TRUE, lpProgressRoutine, lpData ); // incl. mark & extract
		SetPathName( strBasePath );
	}
	else if ( GetParentDoc() != NULL && m_posParent != NULL ) {
		strBasePath = GetParentDoc()->GetFullPathEx( m_posParent, TRUE, lpProgressRoutine, lpData ); // incl. mark & extract
	}
	if ( !m_bScanned )
	{
		m_listArcNew.RemoveAll();
		ScanPathInt( m_listArcNew, strBasePath );	// incl. create+read sub docs m_listZipNew
		if ( (nCancel & nCancelMask) != 0 ) {
			SET_DOC_LOCK(FALSE);
			return FALSE;
		}
		m_bScanned = TRUE;
	}

	TRACE1( "CDocDirZipRoot::ReScanAuto merge %s\n", strBasePath );
	BOOL bNew = CDocDirZipSub::ReScanAuto( bRoot, nCancel, nCancelMask, lpProgressRoutine, lpData );
	SET_DOC_LOCK(FALSE);
	if ( (nCancel & nCancelMask) != 0 )
		return FALSE;

	return bNew;
}

void CDocDirZipRoot::ScanPathInt( CListDirEntries &list, const CString &strPath )
{
	m_arcRoot.PrepareScan( strPath );

	struct _stati64 fs;
	int result = _tstati64( m_arcRoot.GetBasePath(), &fs );
	if ( result != 0 )
		return;			// no file

	TRACE1( "CDocDirZipRoot::ScanPathInt %s\n", strPath );
	CZipArchive zipArc;
	int iMode = CZipArchive::zipOpen;	// 20100112
	if ( (fs.st_mode & _S_IWRITE) == 0 )
	{
		m_bReadOnly = TRUE;
		iMode = CZipArchive::zipOpenReadOnly;
	}
	try {
		zipArc.Open( m_arcRoot.GetBasePath(), iMode );
	}
	catch ( CFileException *e ) {
		TRACE1( "CDocDirZipRoot::ScanPathInt open m_cause=%d\n", e->m_cause );
		if ( e->m_cause == 5 )
		{
			zipArc.Close( 1 );
			m_bReadOnly = TRUE;
			iMode = CZipArchive::zipOpenReadOnly;
			zipArc.Open( m_arcRoot.GetBasePath(), iMode );
		}
		e->Delete();
	}
	zipArc.SetIgnoredConsistencyChecks(CZipArchive::checkCRC);
	int nMax = zipArc.GetCount();		// myZip.GetSize();
	for ( WORD n = 0; n < nMax; ++n )
	{
		CZipFileHeader fhInfo;
		zipArc.GetFileInfo( fhInfo, n );
//		TRACE2( "Z %d %s\n", fhInfo.m_uMethod, fhInfo.GetFileName() );
		CZipFileEntry fe;
		fe.strName = fhInfo.GetFileName();
		fe.dwSize = fhInfo.m_uUncomprSize;
		fe.dt = fhInfo.GetModificationTime();
		fe.crc = fhInfo.m_uCrc32;

		int nFilter = m_arcRoot.GetFilter().GetLength();
		if ( nFilter != 0 ) {
			if ( fe.strName.Left( nFilter ) == m_arcRoot.GetFilter() )
				InsertPath( list, fe.strName.Mid( nFilter ), fe );		// incl. CreateSubDoc and InsertPath there to the corresponding list
		}
		else
			InsertPath( list, fe.strName, fe );		// incl. CreateSubDoc and InsertPath there to the corresponding list
	}	// next n
}

BOOL CDocDirZipRoot::CreateDir()			// assures that this dir is physically present
{
	ASSERT( GetParentDoc() != (CDocDir*)0xfeeefeee );
	if ( GetParentDoc() == NULL )
		return TRUE;			// root dir always exist

	ASSERT( m_posParent != NULL );
	CDirEntry &myde = GetMyDirEntry();
	if ( myde.IsPhysPresent() )
	{
#ifdef _DEBUG
		const CString &strName = myde.GetName();
		CString strPath = myde.GetDoc()->GetFullPathEx( FALSE );
		struct _stati64 fs;
		int result = _tstati64( strPath, &fs );

		ASSERT( result == 0 && (fs.st_mode & _S_IFDIR) == 0 );	// is ZIP !!!
#endif
		return TRUE;
	}
	myde.SetDel( FALSE );

	return GetParentDoc()->CreateDir();
}

CString CDocDirZipRoot::GetFullPathEx( const CString &strName, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine /* = NULL */, LPVOID lpData /* = NULL */ )
{
	ASSERT( m_pArcRoot == this );
	ASSERT( !m_strPathName.IsEmpty() );
	ASSERT( !m_arcRoot.GetBasePath().IsEmpty() );
	CString strPathEx = GetTempPath( strName, bExtract, lpProgressRoutine, lpData );
	strPathEx.Replace( _T("/"), _T("\\") );
	return strPathEx;
}

void CDocDirZipRoot::ExtractFiles( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE1( "CDocDirZipRoot::ExtractFiles %s\n", m_strPathName );
	ASSERT( !m_strPathName.IsEmpty() );
	ASSERT( !m_arcRoot.GetBasePath().IsEmpty() );
	// extract only marked unextracted files and dirs
	MarkDirAny();
	MarkFiles( CDirEntry::Mark |
			   CDirEntry::NoExtr, CDirEntry::Del );
	CStringArray astrFiles;
	__int64 nSize = 0; 
	GetMarkedList( astrFiles, nSize );

	if ( astrFiles.GetCount() != 0 )
	{
		CDocDirZipCallback callback( lpProgressRoutine, lpData, nSize );
		CZipArchive zipArc;
		int iMode = CZipArchive::zipOpen;
		if ( m_bReadOnly )
			iMode = CZipArchive::zipOpenReadOnly;
		CString strPath = GetFullPathEx( TRUE );
		zipArc.Open( m_arcRoot.GetBasePath(), iMode );
		zipArc.SetIgnoredConsistencyChecks(CZipArchive::checkCRC);
		zipArc.EnableFindFast();
		if ( lpProgressRoutine != NULL )
			zipArc.SetCallback( &callback );
		for ( int n = 0; n < astrFiles.GetCount(); ++n )
		{
			CString strFile = astrFiles[n];
			TRACE1( " E %s\n", strFile );
			ZIP_INDEX_TYPE i = zipArc.FindFile( strFile );
			if ( i == ZIP_FILE_INDEX_NOT_FOUND || !zipArc.ExtractFile( i, m_arcRoot.GetTempRoot() ) )
			{
				AfxMessageBox( _T("Can't extract ") + strFile, MB_ICONEXCLAMATION );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}
	}
	SetExtr();
}

CString CDocDirZipRoot::GetTempPath( const CString &strPath, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	CString strTempRoot = m_arcRoot.GetNewTempRoot();

	if ( bExtract )
		ExtractFiles( lpProgressRoutine, lpData );

	if ( strPath.IsEmpty() )
		return strTempRoot;

	return strTempRoot + '\\' + m_arcRoot.GetFilter() + strPath;
}

BOOL CDocDirZipRoot::DoFileSave()
{
	TRACE1( "CDocDirZipRoot::DoFileSave %s\n", m_strPathName );
	ASSERT( !m_strPathName.IsEmpty() );

	if ( !IsModified() )
		return TRUE;

	// save inner docs
	DOCPOS pos = m_list.GetTailPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL )
		{
			if ( !pd->DoFileSave() )
				return FALSE;
		}
		m_list.GetPrev( pos );
	}

	// delete files in zip
	{
		MarkFiles( CDirEntry::Del | CDirEntry::Arch, 0 );
		MarkDirAll();
		CStringArray astrFiles;
		__int64 nSize = 0;
		GetMarkedList( astrFiles, nSize );
		if ( astrFiles.GetSize() != 0 )
		{
			GetTempPath( CString(), FALSE, NULL, NULL );
			CString strPath = GetFullPathEx( FALSE );		// was TRUE
			struct _stati64 fs;
			int result = _tstati64( strPath, &fs );
			if ( result == 0 ) {
				CZipArchive zipArc;
				zipArc.Open( strPath );
				zipArc.RemoveFiles( astrFiles );
				zipArc.Close();
			}
		}
		m_list.SetFlags( CDirEntry::Mark,
						 CDirEntry::Arch | CDirEntry::NoExtr | CDirEntry::Mark,
						 0 );
	}
	// save modified files
	MarkFiles( CDirEntry::Modif, CDirEntry::Del );
	MarkDirAny();
	CStringArray astrFiles;
	__int64 nSize = 0;
	GetMarkedList( astrFiles, nSize );
	if ( astrFiles.GetSize() != 0 )
	{
		ASSERT( !m_arcRoot.GetTempRoot().IsEmpty() );
		CZipArchive zipArc;
		struct _stati64 fs;
		CString strPathName = GetFullPathEx( TRUE );
		int result = _tstati64( strPathName, &fs );
		if ( result == 0 )
			zipArc.Open( strPathName );
		else
			zipArc.Open( strPathName, CZipArchive::zipCreate );
		zipArc.SetIgnoredConsistencyChecks(CZipArchive::checkCRC);
		zipArc.EnableFindFast();
		for ( int n = 0; n < astrFiles.GetCount(); ++n )
		{
			const CString &strFile = astrFiles[n];
			TRACE1( " S %s\n", astrFiles[n] );
			CZipAddNewFileInfo nfi( m_arcRoot.GetTempRoot() + '\\' + strFile, strFile );
			nfi.m_uReplaceIndex = zipArc.FindFile( strFile );
			if ( !zipArc.AddNewFile( nfi ) )
			{
				AfxMessageBox( _T("Can't add ") + strFile, MB_ICONEXCLAMATION );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}
		zipArc.Close();
	}

	m_list.SetFlags( CDirEntry::Modif | CDirEntry::Mark,
					 CDirEntry::Modif | CDirEntry::Mark,
					 CDirEntry::Arch );

	if ( m_posParent != NULL ) {
		CDirEntry &myde = GetMyDirEntry();
		struct _stati64 fs;
		int result = _tstati64( m_strPathName, &fs );
		if ( result == 0 )
		{
			myde.SetDateTime( fs.st_mtime );
			myde.SetFileSize( fs.st_size );
		}
	}

	// reset only inner flags
	pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL )
			pd->CDocDir::SetModifiedFlag( FALSE );

		de.SetModif( FALSE );
		m_list.GetNext( pos );
	}
	return TRUE;
}

void CDocDirZipRoot::Invalidate() 
{
	if ( m_arcRoot.GetTempRoot().IsEmpty() )
		m_bScanned = FALSE;
}


BOOL CDocDirZipRoot::SaveModified( int nSide )
{
	if ( !m_strPathName.IsEmpty() )
	{
		if ( !CDocFileSync::SaveModified( nSide ) )
			return FALSE;
	}
	CDocDir::SetModifiedFlag( FALSE );
	return TRUE;
}

BOOL CDocDirZipRoot::RemoveTempRoot()
{
	TRACE1( "CDocDirZipRoot::RemoveTempRoot() %s\n", m_strName );
	if ( m_arcRoot.GetTempRoot().IsEmpty() )
		return TRUE;

	// MarkFiles( CDocDir::CDirEntry::Flags::Arch, CDocDir::CDirEntry::Flags::NoExtr );
	MarkFiles( 0, CDirEntry::NoExtr );
	MarkDirAny();
	CStringArray astrFiles;
	__int64 nSize = 0;
	GetMarkedList( astrFiles, nSize, TRUE );
	m_arcRoot.RemoveTempRoot( astrFiles );
	m_list.SetFlags( CDirEntry::All,
					 CDirEntry::Modif | CDirEntry::Mark,
					 CDirEntry::NoExtr );

	CDocDir::SetModifiedFlag( FALSE );
	return TRUE;
}

