#include "StdAfx.h"
#include "DocManFileSync.h"
#include <math.h>
#include "DirEntry.h"
#include "MainFrm.h"
#include "Ping.h"
#include "DlgOverwriteNewer.h"

#include "DocDir.h"

IMPLEMENT_DYNCREATE(CDocDir, CDocFileSync)


//##################################################

CDocDir::CMerge::CMerge( CListDirEntries &listL, CListDirEntries &listR ) :
				m_listL( listL ),
				m_listR( listR )
{
	m_posL = m_listL.GetHeadPosition();
	m_posR = m_listR.GetHeadPosition();
}

int CDocDir::CMerge::CompareDoc()
{
	ASSERT( m_posL != NULL || m_posR != NULL );

	if ( m_posR == NULL )
		return -1;

	if ( m_posL == NULL )
		return ( 1 );

	return m_listL.GetAt( m_posL ).Compare( m_listR.GetAt( m_posR ) );
}

void CDocDir::CMerge::GetNext( int nCompare )
{
	if ( nCompare < 1 )
	{
		m_listL.GetNext( m_posL );
	}
	if ( nCompare > -1 )
	{
		m_listR.GetNext( m_posR );
	}
}

void CDocDir::CMerge::CopyR2L()
{
	CDirEntry & de = m_listR.GetAt( m_posR );
	de.SetMark();
	de.SetCopied();
	CDocDir *pd = de.GetDoc();
	de.SetDoc( NULL );
	if ( m_posL == NULL )
		m_posL = m_listL.AddTail( de );
	else
		m_posL = m_listL.InsertBefore( m_posL, de );
	if ( pd != NULL ) {
		CDirEntry &deL = m_listL.GetAt(m_posL);
		deL.SetDoc( pd );
		if ( pd != NULL )
			pd->SetParentPos( m_posL );
	}
}

BOOL CDocDir::CMerge::IsChanged()
{
	CDirEntry & deL = GetDirEntryL();
	CDirEntry & deR = GetDirEntryR();

	if ( deL.GetDateTime() != deR.GetDateTime() )
	{
		CTimeSpan dd = deR.GetDateTime()- deL.GetDateTime();
		LONGLONG ts = dd.GetTotalSeconds();
		if ( ts > 2 || ts < -2 )
			return TRUE;
	}

	return ( deL.GetFileSize() == deR.GetFileSize() );
}

//##################################################

DWORD CDocDir::s_dwLastError = 0;
BOOL CDocDir::s_bAskOverwriteNewer = TRUE;
CString CDocDir::s_strPing;
BOOL CDocDir::s_bSkipComm = TRUE;

CDocDir::CDocDir(void)
{
//	m_nMaxLineLen = 0;
	m_bScanned = FALSE;
}

void CDocDir::Init( const CString &strName, CDocDir *pParent, DOCPOS posParent )
{
	ASSERT( posParent != NULL );
	ASSERT( !strName.IsEmpty() );
	m_strName = strName;
	SetParent( pParent );
	m_posParent = posParent;
	ASSERT( GetMyDirEntry().GetName() == m_strName );
	UpdPathName();
}


void CDocDir::UpdPathName()
{ 
	if ( GetParentDoc() != NULL ) {
		const CString &strParentPathName = GetParentDoc()->GetPathName();
		if ( strParentPathName.Right(1) == _T("\\" ) )
			m_strPathName = strParentPathName + m_strName; 
		else
			m_strPathName = strParentPathName + _T("\\") + m_strName; 
	}
}

CDocDir* CDocDir::CreateSubDoc( const CString &strName, DOCPOS pos ) 
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class
	return NULL;
}

CDocDir::~CDocDir(void)
{
	ASSERT( GetParentDoc() == NULL || GetMyDirEntry().GetName() == m_strName );
	DeleteContents();
}

void CDocDir::DeleteContents()
{
	m_list.RemoveAll();
	CDocFileSync::DeleteContents();
//	m_nMaxLineLen = 0;
//	m_nMaxFileSize = 0;
	m_bScanned = FALSE;
}

BOOL CDocDir::ResetAll()
{
	TRACE1( "CDocDir::ResetAll( %s )\n", m_strName );
//	m_list.RemoveAll();
	m_list.ResetAll();
	m_bScanned = FALSE;

	return CDocFileSync::ResetAll();
}

BOOL CDocDir::RemoveTempRoot(void)
{
	// nothing to do, see CDocDirZipRoot
	return TRUE;
}

BOOL CDocDir::OnOpenDocument( LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE1( "CDocDir::OnOpenDocument( %s )\n", lpszPathName );
#ifdef _DEBUG
	if (IsModified()) {
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");
	}
#endif

//	if ( m_strPathName != lpszPathName ) {		20120504: reset every time (?)
		ResetAll();
		DeleteContents();
//	}

	SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}

// CDocFileSync serialization

void CDocDir::Serialize(CArchive& ar)
{
	ASSERT( FALSE );
}

BOOL CDocDir::Refresh( int nSide, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData)
{
	ASSERT( FALSE );
	return FALSE;
}

void CDocDir::SetModifiedFlag( BOOL bModified /* = TRUE */ )
{
	TRACE3("CDocDir::SetModifiedFlag(%d) <%s,%s>\n", bModified, m_strPathName, m_strName );
	ASSERT_VALID(this);

	CDocFileSync::SetModifiedFlag( bModified );

	if ( bModified )
	{
		ASSERT( !m_strPathName.IsEmpty() );
		if ( GetParentDoc() != NULL && m_posParent != NULL )
		{
			GetParentDoc()->SetModifiedFlag( m_posParent, TRUE );
			GetParentDoc()->SetModifiedFlag();
		}
	}
	else
	{
		DOCPOS pos = m_list.GetHeadPosition();
		while ( pos != NULL )
		{
			CDirEntry &de = m_list.GetAt( pos );
			CDocDir *pd = de.GetDoc();
			if ( pd != NULL )
				pd->SetModifiedFlag( FALSE );

			de.SetModif( FALSE );
			m_list.GetNext( pos );
		}
		if ( m_posParent != NULL ) {
//			ASSERT_VALID(m_pParent);
			GetMyDirEntry().SetModif( FALSE );
		}
	}
}

void CDocDir::SetModifiedFlag( DOCPOS pos, BOOL bModified /* = TRUE */ )
{
	CDirEntry &de = m_list.GetAt(pos);
//	TRACE3("CDocDir::SetModifiedFlag(%s,%d) <%s>\n", de.GetName(), bModified, m_strName );
	de.SetModif( bModified );
	if ( bModified )
	{
//		CDocDir::SetModifiedFlag();		// 20060223

		CString strPath = GetFullPathEx(de.GetName(), FALSE );		// was TRUE
		struct _stati64 fs;
		int result = _tstati64( strPath, &fs );
		if ( result == 0 )
		{
			de.SetFileSize(fs.st_size);
			de.SetDateTime(CTime(fs.st_mtime));
		}
		de.SetCRC(0);
	}
}

BOOL CDocDir::SaveModified( int nSide )
{
	if (!IsModified())
		return TRUE;        // ok to continue

	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL )
			if ( !pd->SaveModified( nSide ) )
				return FALSE;

		de.SetModif( FALSE );
		m_list.GetNext( pos );
	}
	return DoFileSave();
//	return CDocFileSync::SaveModified();
}

#pragma comment( lib, "mpr" )

BOOL CDocDir::CheckConn(const CString &strPath, BOOL bConn)
{ 
	TRACE0("CDocDir::CheckConn\n");
	BOOL bAskConn = TRUE;

	int p = strPath.Find( '\\', 2 );
	if ( p > 2 && strPath[0] == '\\' && strPath[1] == '\\' )
	{
		CString strHost(strPath.Mid( 2, p-2 ));
		if ( strHost == s_strPing )
		{
			bAskConn = FALSE;
		}
		else
		{
			CPing ping;
			if ( !ping.SetHostIP( CStringA(strHost) ) )
				return FALSE;
			if ( !ping.SendEcho() )
			{
				if ( !bConn )
					return FALSE;

				CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
				int nRsp = pFrame->MessageBox( _T("Ping ") + strHost + _T(" timed out.\nTry to connect anyway?"),
					_T("Filesync"), MB_ICONQUESTION | MB_YESNO);
				if (nRsp == IDYES)
					bAskConn = FALSE;
				else
					return FALSE;
			}
			s_strPing = strHost;
		}
	}

	DWORD dwResult;
	DWORD dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;      // 16K is a good size
	DWORD cEntries = -1;         // enumerate all possible entries
	LPNETRESOURCE lpnr = NULL;   // pointer to container resource
	LPNETRESOURCE lpnrLocal;     // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_CONNECTED, // scope
							RESOURCETYPE_DISK,   // type
							0,        // unused
							lpnr,     // NULL first time the function is called
							&hEnum);  // handle to the resource

	if (dwResult != NO_ERROR)
	{ 
		TRACE1( "CDocDir::CheckConn WNetOpenEnum dwResult=%d\n", dwResult );
		//
		// Process errors with an application-defined error handler.
		//
		// NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetOpenEnum");
		return FALSE;
	}
	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);

	do
	{  
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,      // resource handle
										&cEntries,  // defined locally as -1
										lpnrLocal,  // LPNETRESOURCE
										&cbBuffer); // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR)
		{
			int nLen = strPath.Find('\\', 2) + 1;
			for(i = 0; i < cEntries; i++)
			{
				// size_t nLenRemote = wcslen(lpnrLocal[i].lpRemoteName);
				if ( _wcsnicmp( strPath, (const wchar_t *)lpnrLocal[i].lpRemoteName, nLen ) == 0 )
				{
					dwResultEnum = ERROR_HANDLE_EOF;	// force exit
					break;
				}
			}
		}
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
		{
			// Process errors.
			//
			TRACE1( "DocDir CheckConn WNetEnumResource dwResult=%d\n", dwResultEnum );
			// NetErrorHandler(hwnd, dwResultEnum, (LPSTR)"WNetEnumResource");
			break;
		}
	}
	while(dwResultEnum != ERROR_NO_MORE_ITEMS && dwResultEnum != ERROR_HANDLE_EOF);
	//
	// free resources
	//
	GlobalFree((HGLOBAL)lpnrLocal);
	dwResult = WNetCloseEnum(hEnum);

	if ( dwResultEnum == ERROR_HANDLE_EOF) 
		return TRUE;

	if(dwResult != NO_ERROR)
	{ 
		//
		// Process errors.
		//
		TRACE1( "DocDir CheckConn WNetCloseEnum dwResult=%d\n", dwResult );
		// NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetCloseEnum");
		return FALSE;
	}
	else if ( bConn )		// try to connect
	{
		CString strPath2( strPath );
		// cut pure file names
		int p = strPath.ReverseFind( '.' );
		if ( p > 0 ) {
			p = strPath.ReverseFind( '\\' );
			if ( p > 0 )
				strPath2 = strPath.Left( p );
		}

		CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
		if ( bAskConn ) {
			int nRsp = pFrame->MessageBox( _T("Connect to ") + strPath2 + _T("?"), _T("Filesync"), MB_ICONQUESTION | MB_YESNO);
			if (nRsp != IDYES)
				return FALSE;
		}
		NETRESOURCE netres;
		netres.dwType = RESOURCETYPE_DISK;
		netres.lpLocalName = NULL;
		netres.lpRemoteName = (LPTSTR)(LPCTSTR)strPath2;
		netres.lpProvider = NULL;
		TRACE1( "DocDir CheckConn WNetAddConnection2 %s\n", strPath2 );
		dwResult = WNetAddConnection2( &netres, NULL, NULL, CONNECT_INTERACTIVE );
		if ( dwResult == NO_ERROR )
			return TRUE;

		TRACE1( "DocDir CheckConn WNetAddConnection2 dwResult=%d\n", dwResult );
	}
	return FALSE;
}

BOOL CDocDir::PreReScanAuto( int nSide, const CString &strBasePath )
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class
}

BOOL CDocDir::PreReScanAutoBase( int nSide, const CString &strBasePath, USHORT usMode )
{
	TRACE2( "CDocDir::PreReScanAutoBase %s %x\n", strBasePath, (int)usMode );
	if ( !SaveModified( nSide ) )
		return FALSE;

	if ( m_strName.IsEmpty() )
		m_strName = strBasePath;

	CString strPath = strBasePath;

	if ( strPath.Left(2) == _T("\\\\") && ! CheckConn(strPath, TRUE) ) {
		strPath.Empty();
		TRACE0( "CDocDir::PreReScanAutoBase no conn\n" );
	}
	else
	{
		int result = _taccess( strBasePath, 0 );
		if ( result != 0 ) {	// no access
			strPath.Empty();
			TRACE0( "CDocDir::PreReScanAutoBase no base access\n" );
		}
		else
		{
			int nBS = 0;	// backslash counter
			if ( strPath.Left(2) == _T("\\\\") )		// check share names only
			{
				for ( int n=0; n < strPath.GetLength(); ++n )
				{
					if ( strPath[n] == '\\' )
						++nBS;
				}
			}
			if ( nBS != 3 )	// check for dirs excluding path names like \\server\c$
			{
				struct _stati64 fs;
				result = _tstati64( strBasePath, &fs );
	//			if ( result != 0 || (fs.st_mode & _S_IFDIR) == 0 )
				if ( result != 0 || (fs.st_mode & usMode) == 0 ) {
					strPath.Empty();
					TRACE0( "bad mode\n" );
				}
			}
		}
	}
	CDocFileSync::SetPathName( strPath );
	return !strPath.IsEmpty();
}

BOOL CDocDir::ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class
	return FALSE;
}

BOOL CDocDir::CreateDir()			// assures that this dir is physical present
{
	ASSERT( GetParentDoc() != (CDocDir*)0xfeeefeee );
	if ( GetParentDoc() == NULL )
		return TRUE;			// root dir always exist

	ASSERT( m_posParent != NULL );
	CDirEntry &myde = GetMyDirEntry();
//	if ( !myde.IsDel() && !myde.IsNoExtr() )	050317
	if ( myde.IsPhysPresent() )
	{
		CString strPath = myde.GetDoc()->GetFullPathEx( TRUE );
		struct _stati64 fs;
		int result = _tstati64( strPath, &fs );

		if ( result == 0 && (fs.st_mode & _S_IFDIR) != 0 )
			return TRUE;
	}
	return GetParentDoc()->CreateDir( m_posParent );
}

BOOL CDocDir::CreateDir( DOCPOS pos ) 
{
// insert direntry before using this method
//
//	CDirEntry de( TRUE, strName, fs.st_mtime, fs.st_size );
//	POSITION pde = m_list.Find( de );
//	if ( pde == NULL )
//		m_list.Insert( de );
//	else
//		m_list.GetAt( pde ).SetDateTime( COleDateTime( fs.st_mtime ) );

	ASSERT( GetParentDoc() != (CDocDir*)0xfeeefeee );
	ASSERT( pos != NULL );
	if ( ! CreateDir() )	// assure this dir exists
		return FALSE;

	CDirEntry &d = m_list.GetAt( pos );
	const CString &strName = d.GetName();
	CString strPath = d.GetDoc()->GetFullPathEx( TRUE );

	struct _stati64 fs;
	int result = _tstati64( strPath, &fs );

	if ( result == 0 && (fs.st_mode & _S_IFDIR) != 0 )
	{
		d.SetDel(FALSE);
		if (d.GetDateTime() == CTime() )
		{
			d.SetDateTime( CTime( fs.st_mtime ) );
			d.SetFileSize( 0 );
		}
		return TRUE;	// strPath exists
	}

	if ( ! ::CreateDirectory( strPath, NULL ) )
		return FALSE;

	result = _tstati64( strPath, &fs );
	if ( result != 0 )
		return FALSE;

	d.GetDoc()->SetPathName( strPath );	// 20080626
	d.SetDel(FALSE);
	d.SetDateTime( CTime( fs.st_mtime ) );
	d.SetFileSize( 0 );
	return TRUE;
}

BOOL CDocDir::CopyFile( DOCPOS posDest, CDocDir *pDocDirSource, DOCPOS posSource, BOOL bPhysCopy,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	ASSERT( GetParentDoc() != (CDocDir*)0xfeeefeee );
	ASSERT( pDocDirSource != NULL );
	ASSERT( posSource != NULL );
	CDirEntry &deSrc = pDocDirSource->GetList().GetAt(posSource);
	const CString &strName = deSrc.GetName();

	TRACE2("CDocDir::CopyFile() %s %d\n", strName, bPhysCopy );

	if ( !bPhysCopy )
	{
		if ( posDest == NULL )
			posDest = m_list.Insert( CDirEntry( deSrc.IsDir(), strName, deSrc.IsRO(), deSrc.GetDateTime(), deSrc.GetFileSize() ) );
		else
			m_list.GetAt( posDest ).Copy( deSrc, FALSE );

		return TRUE;
	}

	if ( posDest != NULL && s_bAskOverwriteNewer )		// check overwrite
	{
		CDirEntry &deDest = m_list.GetAt(posDest);
		if ( !deDest.IsDir() && deDest.GetDateTime() > deSrc.GetDateTime() )
		{
			CDlgOverwriteNewer dlg;
			dlg.m_strFileName = strName;
			int nRc = dlg.DoModal();
			if ( nRc == IDOK )
				;
			else if ( nRc == IDCANCEL )
				return TRUE;
			else
				s_bAskOverwriteNewer = FALSE;
		}
	}

	if ( deSrc.IsNoExtr() )
		deSrc.SetMark();
	CString strPath = pDocDirSource->GetFullPathEx(strName, deSrc.IsNoExtr());
	deSrc.SetMark(FALSE);
	CString strNewPath = GetFullPathEx(strName, FALSE);

	if ( !CreateDir() )		// assure base dirs are physicaly present
		return FALSE;

	if ( deSrc.IsDir() )
	{
		struct _stati64 fs;
		int result = _tstati64( strNewPath, &fs );
		if ( result != 0 )
		{
			if ( ! ::CreateDirectory( strNewPath, NULL ) )
				return FALSE;
		}
	}
	else
	{
		if ( ! ::CopyFileEx( strPath, strNewPath, lpProgressRoutine, lpData, NULL, 0 ) )
			return FALSE;

		int result = _wchmod( strNewPath, _S_IREAD | _S_IWRITE );
	}

	struct _stati64 fs;
	int result = _tstati64( strPath, &fs );
	if ( result != 0 )
		return FALSE;

	return TRUE;
}

void CDocDir::MakeRW( DOCPOS pos, BOOL b )
{
	MakeRW( GetDirEntry(pos), b );
}

void CDocDir::MakeRW( CDirEntry &de, BOOL b )
{
	const CString &strName = de.GetName();

	TRACE1("CDocDir::MakeRW() %s\n", strName );

	CString strPath = GetFullPathEx(strName, FALSE);
	int mode = b ? _S_IREAD | _S_IWRITE : _S_IREAD;

	int result = _tchmod( strPath, mode );
#ifdef _DEBUG
	if ( result != 0 ) {
		TRACE1("  failed: %s\n", strPath );
	}
#endif

	//SetModifiedFlag( posDest );
}

DOCPOS CDocDir::InsertDummy( BOOL bDir, const CString strName )
{
	CDirEntry de;
	de.SetDir( bDir );
	de.SetName( strName );
	de.SetDel();
	DOCPOS pos = m_list.Find( de );		// 20100817 try to return existing entry
	if ( pos != NULL )
		return pos;
	TRACE1( "CDocDir::InsertDummy %s\n", strName );
	return m_list.Insert( de );
}

void CDocDir::InsertFile( const CDirEntry &de )
{
	ASSERT( de.GetName().Find( '\\' ) < 0 );
	DOCPOS pos = m_list.Find( de );	// FindFile( strName );
	if ( pos != NULL )
	{
		CDirEntry &fde = m_list.GetAt( pos );
		fde.Copy( de );		// de.SetDateTime( dt );
							// de.SetFileSize( fs );
		return;
	}
	pos = m_list.GetHeadPosition();
	m_list.Insert( de );
}

BOOL CDocDir::RemoveFileDir( DOCPOS pos )
{
	if ( pos == NULL )
		return TRUE;

	CDirEntry &de = m_list.GetAt(pos);
	if ( de.IsDel() )
		return TRUE;

	const CString &strName = de.GetName();
	CString strFullPath = GetFullPathEx( strName, FALSE );		// 20120322 was TRUE

	TRACE1("CDocDir::RemoveFileDir() %s\n", strFullPath );
	if ( de.IsDir() )
	{
		if ( !::RemoveDirectory( strFullPath ) )
		{
			s_dwLastError = GetLastError();
			return FALSE;
		}
	}
	else
	{
		if ( !::DeleteFile( strFullPath ) )
		{
			s_dwLastError = GetLastError();
			return FALSE;		// may be RO
		}
	}
	de.SetDel();
	return TRUE;
}

CString CDocDir::GetFullPath( const CString &strDir ) 
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class
	return CString();
}

CString CDocDir::GetFullPathLnk( const CString &strDir )
{
	return GetFullPath( strDir );
}

CString CDocDir::GetFullPathEx( BOOL bExtract )
{
	if ( GetParentDoc() == NULL ) {
		return m_strPathName;
	}
	return GetParentDoc()->GetFullPathEx( m_strName, bExtract ); 
}

CString CDocDir::GetFullPathEx( const CString &strDir, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine /* = NULL */, LPVOID lpData /* = NULL */ )
{
	return GetFullPath( strDir );
}

CString CDocDir::GetFullPathEx( DOCPOS pos, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine /* = NULL */, LPVOID lpData /* = NULL */ )
{
	CDirEntry &de = m_list.GetAt(pos);
	if ( bExtract )	{	// 090402 ??? !!! 100701
		ASSERT(!de.IsMarked());
		de.SetMark();	// extract this file
	}
	CString str = GetFullPathEx( de.GetName(), bExtract, lpProgressRoutine, lpData );
	de.SetMark(FALSE);
	return str;
}

CDocFileSync* CDocDir::OpenDocumentFile( DOCPOS pos ) 
{
	return OpenDocumentFile( pos, NULL, NULL, NULL );
}

CDocFileSync* CDocDir::OpenDocumentFile( DOCPOS pos, CDocTemplFileSync* pTemplate, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData ) 
{
	ASSERT( pos != (DOCPOS)0xfeeefeee && (pos != NULL || pTemplate != NULL) );
//	AfxGetApp()->OpenDocumentFile( strFullPath );
//	CDirEntry &de = m_list.GetAt(pos);
//	CString strFullPath = GetPathNameView() + _T("\\") + de.GetName();
	CString strFullPathEx;
	if ( pos != NULL )
		strFullPathEx = GetFullPathEx( pos, TRUE, lpProgressRoutine, lpData );
	CDocManFileSync *pDM = (CDocManFileSync *)AfxGetApp()->m_pDocManager;
	CDocFileSync *pDocNew = (CDocFileSync *)pDM->OpenDocumentFile( strFullPathEx, pTemplate );
	if ( pDocNew != NULL && pos != NULL )
	{
		pDocNew->SetParentAndPos( this, pos );
	}
//	pDocNew->SetPathNameView( strFullPath );
	return pDocNew;
}

int CDocDir::GetSubIcon()
{
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class
	return 0;
}

#ifdef  __cplusplus
extern "C" {
#endif
extern ULONG crc32( ULONG crc, LPCSTR buf, size_t len);
#ifdef  __cplusplus
}
#endif

ULONG CDocDir::ComputeCRC( DOCPOS pos, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	ASSERT( pos != NULL );
	CDirEntry &de = m_list.GetAt(pos);
	ULONG crc = de.GetCRC();
	if ( crc != 0 )
		return crc;		// done

	if ( de.GetFileSize() == 0 )
		return 1;

	CString strFullPath = GetFullPathEx( de.GetName(), TRUE );
	try {
		CFile file( strFullPath, CFile::modeRead | CFile::shareDenyWrite );
		LARGE_INTEGER nTotal;
		LARGE_INTEGER nTransferred;
		LARGE_INTEGER nStreamSize;
		LARGE_INTEGER nStreamTransf;
		nTotal.QuadPart = file.GetLength();
		nTransferred.QuadPart = 0;
		DWORD nBufRead = 0;
		char buf[4096];
		UINT nRead = file.Read( &buf, sizeof(buf) );
		while ( nBufRead < 0x990 && nRead != 0 )		// compare < 10MB
		{
			if ( lpProgressRoutine != NULL ) {
				DWORD dwReason = CALLBACK_STREAM_SWITCH;
				nStreamSize.QuadPart = 4096;
				nStreamTransf.QuadPart = nRead;
				nTransferred.QuadPart += nRead;
				DWORD rc = (*lpProgressRoutine)( nTotal, nTransferred, nStreamSize, nStreamTransf, nBufRead, dwReason, NULL, NULL, lpData );
			}
			++nBufRead;
			crc = crc32( crc, buf, nRead );
			nRead = file.Read( &buf, sizeof(buf) );
		}
		de.SetCRC( crc );
	} catch ( CFileException *pe )
	{
//		ASSERT( FALSE );
		pe->Delete();
		crc = 0;
	}
	return crc;
}

ULONG CDocDir::CRC32T( ULONG crc, LPCSTR buf, size_t len, BOOL &bInComm)
{
	if ( s_bSkipComm )
	{
		const char *p = buf-1;
		if ( bInComm ) {
//			TRACE3( "# %d %d %.20S\n", crc, len, buf );
			while ( p != NULL && (p+1) < (buf+len) ) {
				p = strchr( p+1, '/' );
				if ( p != NULL && p < (buf+len) && *(p-1) == '*' ) {	// handle end comm
					bInComm = FALSE;
					len -= p-buf+1;
					if ( len > 0 )
						return CRC32T( crc, p+1, len, bInComm );
					break;
				}
			}
			return crc;
		}
		else {
			while ( p != NULL && (p+1) < (buf+len) ) {
				p = strchr( p+1, '*' );
				if ( p != NULL && p < (buf+len) && *(p-1) == '/' ) {	// handle start comm
					bInComm = TRUE;
					crc = crc32( crc, buf, p-buf+1 );
					len -= p-buf+1;
					if ( len > 0 )
						return CRC32T( crc, p+1, len, bInComm );
					else
						return crc;
				}
			}
			return crc32( crc, buf, len );
		}
	}
	else
		return crc32( crc, buf, len );
}

ULONG CDocDir::ComputeCRCText( DOCPOS pos, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	// TODO: still assuming 8 bit encoding

	ASSERT( pos != NULL );
	CDirEntry &de = m_list.GetAt(pos);
	ULONG crc = de.GetCRC();
	if ( crc != 0 )
		return crc;		// done

	if ( de.GetFileSize() == 0 )
		return 1;

	CString strFullPath = GetFullPathEx( de.GetName(), TRUE );
	TRACE1( "CDocDir::ComputeCRCText %s\n", strFullPath );
	try {
		CFile file( strFullPath, CFile::modeRead | CFile::shareDenyWrite );

		BOOL bInComm = FALSE;
		const int nBufSz = 4096;
		char buf[1+nBufSz+1];	// [prev] [contents] [fin]
		buf[0] = '\0';
		LARGE_INTEGER nTotal;
		LARGE_INTEGER nTransferred;
		LARGE_INTEGER nStreamSize;
		LARGE_INTEGER nStreamTransf;
		nTotal.QuadPart = file.GetLength();
		nTransferred.QuadPart = 0;
		DWORD nBufRead = 0;
		UINT nRead = file.Read( buf+1, nBufSz );
		while ( nBufRead < 0x990 && nRead != 0 )
		{
			if ( lpProgressRoutine != NULL ) {
				DWORD dwReason = CALLBACK_STREAM_SWITCH;
				nStreamSize.QuadPart = nBufSz;
				nStreamTransf.QuadPart = nRead;
				nTransferred.QuadPart += nRead;
				DWORD rc = (*lpProgressRoutine)( nTotal, nTransferred, nStreamSize, nStreamTransf, nBufRead, dwReason, NULL, NULL, lpData );
			}
			++nBufRead;
			buf[nRead+1] = '\0';	// fin
			char *s = buf+1;
			char *p = strchr( s, '\n' );
			while ( p != NULL )
			{
				int nLen = (int)(p - s);
				if ( *(p-1) == '\r' )			// exclude '\r'
					--nLen;
				if ( nLen > 0 )
					crc = CRC32T( crc, s, nLen, bInComm );
				crc = crc32( crc, "\n", 1 );
				s = p + 1;
				p = strchr( s, '\n' );
			}
			int nLen = (int)(buf+nRead+1 - s);
			if ( *(buf+nRead) == '\r' )
				--nLen;
			if ( nLen > 0 )
				crc = CRC32T( crc, s, nLen, bInComm );
			buf[0] = buf[nRead];			// save last char for \r detection
			nRead = file.Read( buf+1, nBufSz );
		}
		if ( crc != 0 ) {
			if ( buf[0] != '\n' )
				crc = crc32( crc, "\n", 1 );
			de.SetCRC( crc );
		}
	} catch ( CFileException *pe )
	{
//		ASSERT( FALSE );
		pe->Delete();
		crc = 0;
	}
	return crc;
}

/*
BOOL CDocDir::CompareBinary( const CString &strFileName )
{
//	if ( m_pOther == NULL )		// todo: ?????
		return FALSE;
	CString strPathOwn   = GetFullPath( strFileName );
	CString strPathOther;	// todo: = m_pOther->GetFullPath( strFileName );
	CFile fileOwn( strPathOwn, CFile::modeRead );
	CFile fileOther( strPathOther, CFile::modeRead );
	char bufOwn[4096];
	char bufOther[4096];
	UINT nReadOwn = fileOwn.Read( &bufOwn, sizeof(bufOwn) );
	UINT nReadOther = fileOther.Read( &bufOther, sizeof(bufOther) );
	BOOL bEqual = (nReadOwn == nReadOther);
	while ( bEqual && nReadOwn != 0 )
	{
		bEqual = memcmp( bufOwn, bufOther, sizeof(bufOwn) ) == 0;
		if ( !bEqual )
			break;
		nReadOwn = fileOwn.Read( &bufOwn, sizeof(bufOwn) );
		nReadOther = fileOther.Read( &bufOther, sizeof(bufOther) );
		bEqual = (nReadOwn == nReadOther);
	} 
	return bEqual;
}
*/

BOOL CDocDir::DoFileSave()
{
	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL && pd->IsModified() )
		{
			if ( !pd->DoFileSave() )
				return FALSE;
		}
		m_list.GetNext( pos );
	}
	SetModifiedFlag( FALSE );
	return TRUE;
}

BOOL CDocDir::HasAnyDocs() const
{
	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		const CDirEntry &de = m_list.GetAt( pos );
		if ( !de.IsDel() )
			if ( de.GetDoc() != NULL )
				return TRUE;
		m_list.GetNext( pos );
	}
	return FALSE;
}

BOOL CDocDir::MarkDirAny()
{
	// mark directory if any contents of this dir is marked

	BOOL bMark = FALSE;
	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL && !pd->IsRoot() )		// was: IsKindOf( RUNTIME_CLASS( CDocDirIsoRoot )))
		{
			if ( pd->MarkDirAny() )
			{
				de.SetMark();
				bMark = TRUE;
			}
		}
		else if ( de.IsMarked() )
		{
			return TRUE;	// assuming all dirs are above normal files
		}
		m_list.GetNext( pos );
	}
	return bMark;
}

BOOL CDocDir::MarkDirAll()
{
	// mark directory if entire contents of this dir is marked

	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL )
		{
			if ( pd->MarkDirAll() )
				de.SetMark();
			else
				return FALSE;
		}
		else if ( !de.IsMarked() )
		{
			return FALSE;
		}
		m_list.GetNext( pos );
	}
	return TRUE;
}

void CDocDir::SetExtr()
{
	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL && de.IsDir() )
		{
			pd->SetExtr();
		}
		else if ( de.IsMarked() )
		{
			de.SetMark( FALSE );
			de.SetNoExtr( FALSE );
		}
		m_list.GetNext( pos );
	}
}

void CDocDir::MarkFiles( DWORD maskIncl, DWORD maskExcl )
{
	TRACE3( "CDocDir::MarkFiles %4.4x %4.4x %s\n", maskIncl, maskExcl, m_strName );
	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
		de.SetMark( de.AreAll(maskIncl) && de.IsNotAny(maskExcl) );
		TRACE2( " M %4.4x %s\n", de.GetFlags(), de.GetName() );
		CDocDir *pd = de.GetDoc();
		if ( pd != NULL )
		{
			pd->MarkFiles( maskIncl, maskExcl );
		}
		m_list.GetNext( pos );
	}
}

#ifdef _DEBUG
void CDocDir::AssertValid() const
{
	CDocFileSync::AssertValid();
	m_list.AssertValid();
}
#endif
