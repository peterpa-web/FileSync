#pragma once
#include "DocDirIsoSub.h"
#include "ArcRoot.h"

class CDocDirIsoRoot :
	public CDocDirIsoSub
{
	DECLARE_DYNCREATE(CDocDirIsoRoot)

public:
	CDocDirIsoRoot(void);
	static CDocDirIsoRoot* New(const CString &strName, CDocDir *pParent, DOCPOS posParent);
	~CDocDirIsoRoot(void);

    CDocDirIsoRoot & operator =(const CDocDirIsoRoot &src) { ASSERT(FALSE); return *this; }
	virtual void Delete();

protected:
//	CString m_strTempRoot;
//	CString m_strFilter;
	CArcRoot m_arcRoot;

public:
	virtual void DeleteContents();
	virtual BOOL CheckPath();
	virtual CString GetFullPathEx( BOOL bExtract ) { return CDocDir::GetFullPathEx( bExtract ); }
	virtual CString GetFullPathEx( const CString &strName, BOOL bExtract,
		LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL );
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData);
	virtual void SetParentAndPos( CDocDir *pParent, DOCPOS pos ) {}
	virtual BOOL CreateDir();
	virtual BOOL PreReScanAuto( int nSide, const CString &strBasePath );
	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual BOOL ResetAll();
	virtual BOOL DoFileSave();
	virtual void Invalidate();
	virtual BOOL SaveModified( int nSide );
	virtual BOOL IsRoot() const { return TRUE; }
	virtual CString GetTempPath( const CString &strPath, BOOL bExtract,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual void SetBasePath(LPCTSTR lpszPathName) { m_arcRoot.SetBasePath( lpszPathName ); }
	const CString GetFilter() const { return m_arcRoot.GetFilter(); }
	const CString GetBasePath() const { return m_arcRoot.GetBasePath(); }

protected:
	virtual BOOL RemoveTempRoot();		// remove temp directory

public:
	void ExtractFiles( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );

private:
    static CStorage<CDocDirIsoRoot> s_storeDocs;
	void ScanPathInt( CListDirEntries &list, const CString &strPath );
};
