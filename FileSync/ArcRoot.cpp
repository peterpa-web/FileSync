#include "StdAfx.h"
#include <sys/stat.h>

#include "ArcRoot.h"


CArcRoot::CArcRoot(void)
{
}


CArcRoot::~CArcRoot(void)
{
//	TRACE2( "CArcRoot::~CArcRoot() %x %s\n", this, m_strName );
	DeleteContents();
}

void CArcRoot::DeleteContents()
{ 
	ASSERT( m_strTempRoot.IsEmpty() );
	m_strBasePath.Empty();
	m_strFilter.Empty();
}

void CArcRoot::PrepareScan( const CString &strPath )
{
	if ( !m_strBasePath.IsEmpty() ) {
		m_strFilter = strPath.Right( strPath.GetLength() - m_strBasePath.GetLength() - 1 );
		if ( !m_strFilter.IsEmpty() )
			m_strFilter += _T("\\");
	}
	else
		m_strBasePath = strPath;
}

CString CArcRoot::GetNewTempRoot()
{
	if ( m_strTempRoot.IsEmpty() )
	{
		CString strTemp;
		LPTSTR lpTempFileName = strTemp.GetBuffer( MAX_PATH );
		DWORD dwRc = ::GetTempPath( MAX_PATH, lpTempFileName );
		if ( dwRc == 0 )
		{
			AfxThrowFileException( CFileException::genericException, GetLastError(), _T("(tmp)") );
		}
		dwRc = GetTempFileName( lpTempFileName, _T("FileSync"), (UINT)this, lpTempFileName );
		if ( dwRc == 0 )
		{
			AfxThrowFileException( CFileException::genericException, GetLastError(), lpTempFileName );
		}
		strTemp.ReleaseBuffer();

		// check path and increment if necessary
		int nTemp = 0;
		CString strN;
		struct _stati64 fs;
		int result = _tstati64( strTemp, &fs );

		while ( result == 0 )		// already exists
		{
			++nTemp;
			strN.Format( _T("%x"), nTemp );
			result = _tstati64( strTemp + strN, &fs );
		}
		m_strTempRoot = strTemp + strN;

		if ( ! ::CreateDirectory( m_strTempRoot, NULL ) )
		{
			AfxThrowFileException( CFileException::genericException, GetLastError(), m_strTempRoot );
		}
		TRACE1( "CArcRoot::GetTempPath m_strTempRoot=%s\n", m_strTempRoot );
	}

//	if ( bExtract )
//		ExtractFiles( lpProgressRoutine, lpData );

//	if ( strPath.IsEmpty() )
//		return m_strTempRoot;

//	return m_strTempRoot + '\\' + m_strFilter + strPath;
	return m_strTempRoot;
}

BOOL CArcRoot::RemoveTempRoot( CStringArray &astrFiles )
{
	TRACE1( "CArcRoot::RemoveTempRoot() %s\n",  m_strTempRoot );

	if ( m_strTempRoot.IsEmpty() )
		return TRUE;

//	MarkFiles( 0, CDirEntry::NoExtr );
//	MarkDirAny();
//	CStringArray astrFiles;
//	__int64 nSize = 0;
//	GetMarkedList( astrFiles, nSize, TRUE );
	int n;
	for ( n = 0; n < astrFiles.GetSize(); ++n )
	{
		CString strPath = m_strTempRoot + '\\' + CString(astrFiles.GetAt( n ));
		if ( strPath.Right(1) != '\\' )
		{
			struct _stati64 fs;
			int result = _tstati64( strPath, &fs );
			if ( result == 0 )
			{
				TRACE1( " del %s\n",  strPath );
				if ( !DeleteFile( strPath ) )
				{
					AfxMessageBox( _T("Can't delete ") + strPath, MB_ICONEXCLAMATION );
				}
			}
		}
	}

	while ( --n >= 0 )
	{
		CString strPath = m_strTempRoot + '\\' + CString(astrFiles.GetAt( n ));
		if ( strPath.Right(1) == '\\' )
		{
			strPath = strPath.Left(strPath.GetLength()-1);
			struct _stati64 fs;
			int result = _tstati64( strPath, &fs );
			if ( result == 0 )
			{
				TRACE1( " rem %s\n",  strPath );
				if ( !RemoveDirectory( strPath ) )
				{
					DWORD dwErr = GetLastError();
					AfxMessageBox( _T("Can't remove ") + strPath, MB_ICONEXCLAMATION );
				}
			}
		}
	}
//	m_list.SetFlags( CDirEntry::All,
//					 CDirEntry::Modif | CDirEntry::Mark,
//					 CDirEntry::NoExtr );

	// remove filter dirs
	CString strF;
	if ( !m_strFilter.IsEmpty() )
		strF = m_strFilter.Left( m_strFilter.GetLength()-1 );		// cut trailing '\';
	while ( !strF.IsEmpty() )
	{
		RemoveDirectory( m_strTempRoot + '\\' + strF );
		int p = strF.ReverseFind( '\\' );
		if ( p < 0 )
			break;
		strF = strF.Left( p );
	}
	// remove main
	if ( !RemoveDirectory( m_strTempRoot ) )
	{
		DWORD dwErr = GetLastError();
		AfxMessageBox( _T("Can't remove ") + m_strTempRoot, MB_ICONEXCLAMATION );
	}
	m_strTempRoot.Empty();
//	CDocDir::SetModifiedFlag( FALSE );
	return TRUE;
}

BOOL CArcRoot::CreateTempSubDir( const CString &strPathName )			// assures that this dir is physically present
{
	int p = strPathName.ReverseFind( '\\' );
	if ( p < 0 )
		return FALSE;	// bad path
	CString strPath = strPathName.Left( p );

	struct _stati64 fs;
	int result = _tstati64( strPath, &fs );

	if ( result == 0 && (fs.st_mode & _S_IFDIR) == _S_IFDIR )
		return TRUE;	// dir is present

	if ( !CreateTempSubDir( strPath ) )
		return FALSE;

	if ( ! ::CreateDirectory( strPath, NULL ) )
	{
		AfxThrowFileException( CFileException::genericException, GetLastError(), strPath );
	}
	return TRUE;
}
