#pragma once
#include "afxtempl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "DocDir.h"

class CDocDirNative :
    public CDocDir
{
	DECLARE_DYNCREATE(CDocDirNative)

public:
	CDocDirNative(void);
	virtual CDocDir* CreateSubDoc( const CString &strName, DOCPOS pos );
    CDocDirNative & operator =(const CDocDirNative &src) { ASSERT(FALSE); return *this; }
	virtual ~CDocDirNative(void);

	virtual void Delete();
	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o

	virtual BOOL CheckPath() { return TRUE; }
	virtual CString GetFullPath( const CString &strDir );
	virtual CString GetFullPathLnk( const CString &strDir );

	virtual void SetBasePath(LPCTSTR lpszPathName) { }
//	virtual void ReScanPath( int nSide, BOOL bRoot, const int &nCancel );
	virtual BOOL PreReScanAuto( int nSide, const CString &strBasePath );
	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual int GetSubIcon();

	CListDirEntries & GetListNew() { return m_listNew; }
	DWORD GetLastScanTickCount() { return m_dwLastScan; }

	static void ToggleHidePJ() { s_bHidePJ = !s_bHidePJ; }
	static BOOL IsHidePJ() { return s_bHidePJ; }
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

private:
	struct _stati64 m_fs;
	CListDirEntries m_listNew;
	static CStorage<CDocDirNative> s_storeDocs;
	DWORD m_dwLastScan;
	static BOOL s_bHidePJ;

	void ScanPathInt( CListDirEntries &list, const CString &strPath, const int &nCancel, int nCancelMask, BOOL bRoot );
	void GetFileStatus( BOOL bDir );
	static BOOL CreateDir( CString strDir );

};
