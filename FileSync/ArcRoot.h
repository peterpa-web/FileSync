#pragma once
#include "stdafx.h"
//#include "DocDirArchive.h"

class CArcRoot
{
public:
	CArcRoot(void);
	~CArcRoot(void);

protected:
	CString m_strBasePath;
	CString m_strTempRoot;
	CString m_strFilter;

public:
	void SetBasePath( LPCTSTR lpszPathName ) { m_strBasePath = lpszPathName; }
	const CString GetBasePath() const { return m_strBasePath; }
	void SetFilter( const CString &str ) { m_strFilter = str; }
	const CString GetFilter() const { return m_strFilter; }
	const CString GetTempRoot() const { return m_strTempRoot; }

	void DeleteContents();
	void PrepareScan( const CString &strPath );
	CString GetNewTempRoot();
	BOOL RemoveTempRoot( CStringArray &astrFiles );		// remove temp directory & contained files
	BOOL CreateTempSubDir( const CString &strPathName );
};
