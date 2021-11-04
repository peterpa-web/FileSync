#include "StdAfx.h"
#pragma comment(lib, "CDVD.lib")
//#include <sys/stat.h>
#include "ViewFileSync.h"

#include "DocDirIsoRoot.h"

IMPLEMENT_DYNCREATE(CDocDirIsoRoot, CDocDirIsoSub)

// todo
//BOOL CDirEntry::IsIso() const
//{
//	if ( m_pd == NULL )
//		return FALSE;
//	return m_pd->IsKindOf(RUNTIME_CLASS( CDocDirIsoRoot ));
//}

// #########################

CStorage<CDocDirIsoRoot> CDocDirIsoRoot::s_storeDocs( 10, _T("CDocDirIsoRoot") );

CDocDirIsoRoot::CDocDirIsoRoot(void)
{
//	m_bExtracted = FALSE;
	m_pArcRoot = this;
}

CDocDirIsoRoot* CDocDirIsoRoot::New( const CString &strName, CDocDir *pParent, DOCPOS posParent ) 
{
    ASSERT( posParent != NULL );

	POSITION posDoc = s_storeDocs.New();
    CDocDirIsoRoot *pDoc = s_storeDocs.GetPtrAt(posDoc);
	pDoc->m_posStorage = posDoc;
	TRACE2( "CDocDirIsoRoot::New( %s ) %x\n", strName, pDoc );
	pDoc->Init( strName, pParent, posParent );
    return pDoc;
}

CDocDirIsoRoot::~CDocDirIsoRoot(void)
{
	TRACE2( "CDocDirIsoRoot::~CDocDirIsoRoot() %x %s\n", this, m_strName );
	ASSERT( !IsModified() );
	DeleteContents();	// remove subitems & temp dir
}

void CDocDirIsoRoot::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeDocs.DeleteAt( pos );
    }
    else
        delete this;
}

void CDocDirIsoRoot::DeleteContents()
{
	TRACE1( "CDocDirIsoRoot::DeleteContents() %s\n", m_strName );
	CDocDirIsoSub::DeleteContents();
    m_arcRoot.DeleteContents();
//	ResetAll();
}

BOOL CDocDirIsoRoot::ResetAll()
{
	if ( !RemoveTempRoot() )
		return FALSE;

	if ( CDocDirIsoSub::ResetAll() ) {
//		CDocDir::DeleteContents();
		return TRUE;
	}
	return FALSE;
}

BOOL CDocDirIsoRoot::CheckPath()
{
	int result = _taccess( m_arcRoot.GetBasePath(), 0 );
	return ( result == 0 );
}

BOOL CDocDirIsoRoot::OnOpenDocument(LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData)
{
	TRACE2( "CDocDirIsoRoot::OnOpenDocument( %s ) %s\n", lpszPathName, 
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

BOOL CDocDirIsoRoot::PreReScanAuto( int nSide, const CString &strBasePath )
{
	return PreReScanAutoBase( nSide, m_arcRoot.GetBasePath(), _S_IFREG );
}

BOOL CDocDirIsoRoot::ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
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
		ScanPathInt( m_listArcNew, strBasePath );	// incl. create+read sub docs m_listIsoNew
		if ( (nCancel & nCancelMask) != 0 ) {
			SET_DOC_LOCK(FALSE);
			return FALSE;
		}
		m_bScanned = TRUE;
	}

	TRACE1( "CDocDirIsoRoot::ReScanAuto merge %s\n", strBasePath );
	BOOL bNew = CDocDirIsoSub::ReScanAuto( bRoot, nCancel, nCancelMask, lpProgressRoutine, lpData );
	SET_DOC_LOCK(FALSE);
	if ( (nCancel & nCancelMask) != 0 )
		return FALSE;

	return bNew;
}

void CDocDirIsoRoot::ScanPathInt( CListDirEntries &list, const CString &strPath )
{
	m_arcRoot.PrepareScan( strPath );

	struct _stati64 fs;
	int result = _tstati64( m_arcRoot.GetBasePath(), &fs );
	if ( result != 0 )
		return;			// no file

	TRACE1( "CDocDirIsoRoot::ScanPathInt %s\n", strPath );
	IsoNativeFile isoArc;
//	int iMode = CZipArchive::zipOpen;
//	if ( (fs.st_mode & _S_IWRITE) == 0 )
//	{
		m_bReadOnly = TRUE;
//		iMode = CZipArchive::zipOpenReadOnly;
//	}
	try {
        isoArc.Open( m_arcRoot.GetBasePath() );     // , iMode
	}
	catch ( CFileException *e ) {
		TRACE1( "CDocDirIsoRoot::ScanPathInt open m_cause=%d\n", e->m_cause );
		if ( e->m_cause == 5 )
		{
			m_bReadOnly = TRUE;
//			iMode = CZipArchive::zipOpenReadOnly;
			isoArc.Open( strPath );
		}
		e->Delete();
	}
	IsoDirectory root(isoArc);
	CString strFilter = m_arcRoot.GetFilter();
	if ( strFilter.IsEmpty() )
		InsertPath( list, root );		// incl. CreateSubDoc and InsertPath there to the corresponding list
	else {
		IsoFileDescriptor fd = root.FindFile( strFilter );
		//while ( !strFilter.IsEmpty() )
		//IsoDirectory &isoDir
		//if ( fe.strName.Left( nFilter ) == m_strFilter )
		//const IsoFileDescriptor &fd = isoDir.files.GetAt(n);
		//if ( !fd.IsDir() ) {
		IsoDirectory isoSubDir( root.GetReader(), fd );
		InsertPath( list, isoSubDir );		// incl. CreateSubDoc and InsertPath there to the corresponding list
	}

}

BOOL CDocDirIsoRoot::CreateDir()			// assures that this dir is physically present
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

		ASSERT( result == 0 && (fs.st_mode & _S_IFDIR) == 0 );	// is ISO file !!!
#endif
		return TRUE;
	}
	myde.SetDel( FALSE );

	return GetParentDoc()->CreateDir();
}

CString CDocDirIsoRoot::GetFullPathEx( const CString &strName, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine /* = NULL */, LPVOID lpData /* = NULL */ )
{
	ASSERT( m_pArcRoot == this );
	ASSERT( !m_strPathName.IsEmpty() );
	ASSERT( !m_arcRoot.GetBasePath().IsEmpty() );
	CString strPathEx = GetTempPath( strName, bExtract, lpProgressRoutine, lpData );
	strPathEx.Replace( _T("/"), _T("\\") );
	return strPathEx;
}


void CDocDirIsoRoot::ExtractFiles( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE1( "CDocDirIsoRoot::ExtractFiles %s\n", m_strPathName );
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
		IsoNativeFile isoArc;
//		if ( m_bReadOnly )
//			iMode = CZipArchive::zipOpenReadOnly;
		isoArc.Open( m_arcRoot.GetBasePath() );		// , iMode
		IsoDirectory root(isoArc);
		for ( int n = 0; n < astrFiles.GetCount(); ++n )
		{
			CString strPath = astrFiles[n];
			TRACE1( " E %s\n", strPath );
			if ( !m_arcRoot.CreateTempSubDir( m_arcRoot.GetTempRoot() + '\\' + strPath ) ||
				 !root.Extract( strPath, m_arcRoot.GetTempRoot(), lpProgressRoutine, lpData ) )
			{
				AfxMessageBox( _T("Can't extract ") + strPath, MB_ICONEXCLAMATION );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}
	}
	SetExtr();
}

CString CDocDirIsoRoot::GetTempPath( const CString &strPath, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	CString strTempRoot = m_arcRoot.GetNewTempRoot();

	if ( bExtract )
		ExtractFiles( lpProgressRoutine, lpData );

	if ( strPath.IsEmpty() )
		return strTempRoot;

	return strTempRoot + '\\' + m_arcRoot.GetFilter() + strPath;
}

BOOL CDocDirIsoRoot::DoFileSave()
{
	TRACE1( "CDocDirIsoRoot::DoFileSave %s\n", m_strPathName );
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

	// delete files in iso
	{
		MarkFiles( CDirEntry::Del | CDirEntry::Arch, 0 );
		MarkDirAll();
		CStringArray astrFiles;
		__int64 nSize = 0;
		GetMarkedList( astrFiles, nSize );
		if ( astrFiles.GetSize() != 0 )
		{
			GetTempPath( CString(), FALSE, NULL, NULL );
			CString strPath = GetFullPathEx( FALSE );	// 20120322 was TRUE
			struct _stati64 fs;
			int result = _tstati64( strPath, &fs );
			if ( result == 0 ) {
				/*
				CZipArchive isoArc;
				isoArc.Open( GetFullPathEx() );
				isoArc.RemoveFiles( astrFiles );
				isoArc.Close();
				*/
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
		ASSERT(FALSE);
		/*
		ASSERT( !m_strTempRoot.IsEmpty() );
		CZipArchive isoArc;
		struct _stati64 fs;
		CString strPathName = GetFullPathEx();
		int result = _tstati64( strPathName, &fs );
		if ( result == 0 )
			isoArc.Open( strPathName );
		else
			isoArc.Open( strPathName, CZipArchive::zipCreate );
		isoArc.SetIgnoredConsistencyChecks(CZipArchive::checkCRC);
		isoArc.EnableFindFast();
		for ( int n = 0; n < astrFiles.GetCount(); ++n )
		{
			const CString &strFile = astrFiles[n];
			TRACE1( " S %s\n", astrFiles[n] );
			CZipAddNewFileInfo nfi( m_strTempRoot + '\\' + strFile, strFile );
			nfi.m_uReplaceIndex = isoArc.FindFile( strFile );
			if ( !isoArc.AddNewFile( nfi ) )
			{
				AfxMessageBox( _T("Can't add ") + strFile, MB_ICONEXCLAMATION );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}
		isoArc.Close();
		*/
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

void CDocDirIsoRoot::Invalidate() 
{
	if ( m_arcRoot.GetTempRoot().IsEmpty() )
		m_bScanned = FALSE;
}


BOOL CDocDirIsoRoot::SaveModified( int nSide )
{
	if ( !m_strPathName.IsEmpty() )
	{
		if ( !CDocFileSync::SaveModified( nSide ) )
			return FALSE;
	}
	CDocDir::SetModifiedFlag( FALSE );
	return TRUE;
}

BOOL CDocDirIsoRoot::RemoveTempRoot()
{
	TRACE1( "CDocDirIsoRoot::RemoveTempRoot() %s\n",  m_strName );
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

