#pragma once
#include "afxtempl.h"
#include "DocDir.h"

class CDocDirArchive :
	public CDocDir
{
	DECLARE_DYNCREATE(CDocDirArchive)

public:
	CDocDirArchive(void);
//    virtual CDocDir* CreateSubDoc( const CString &strName, DOCPOS pos );
	~CDocDirArchive(void);

    CDocDirArchive & operator =(const CDocDirArchive &src) { ASSERT(FALSE); return *this; }
//	virtual void Delete();
	virtual void DeleteContents();
	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual CString GetFullPath( const CString &strDir );
	virtual CString GetFullPathEx( const CString &strName, BOOL bExtract=TRUE,
		LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL );
	virtual void SetModifiedFlag( DOCPOS pos, BOOL bModified = TRUE );
	virtual BOOL CopyFile( DOCPOS posDest, CDocDir *pDocDirSource, DOCPOS posSource, BOOL bPhysCopy,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );

	virtual int GetSubIcon() { return IconDir; }
//	virtual BOOL DoFileSave();
	virtual void Invalidate() { }	// don't m_bScanned = FALSE;

	virtual BOOL RemoveFileDir( DOCPOS pos );
	virtual const CString GetFilter() const { return CString(); }
	virtual CString GetTempPath( const CString &strPath, BOOL bExtract,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData ) { ASSERT(FALSE); return CString(); }
	void GetMarkedList( CStringArray &astrFiles, __int64 &nSize, BOOL bInclDirs=FALSE );

protected:
	CDocDirArchive *m_pArcRoot;		// base document; i.e. of class CDocDirZipRoot
	CListDirEntries m_listArcNew;

	CString GetSubPath();
	void AssureDirArcNew();

private:
//    static CStorage<CDocDirArchive> s_storeDocs;

};
