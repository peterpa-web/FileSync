#pragma once
#include "afxtempl.h"
#include "DocDirArchive.h"
#include "IsoNativeFile.h"

class CDocDirIsoRoot;

class CDocDirIsoSub :
	public CDocDirArchive
{
	DECLARE_DYNCREATE(CDocDirIsoSub)

public:
	CDocDirIsoSub(void);
    virtual CDocDir* CreateSubDoc( const CString &strName, DOCPOS pos );
	~CDocDirIsoSub(void);

    CDocDirIsoSub & operator =(const CDocDirIsoSub &src) { ASSERT(FALSE); return *this; }
	virtual void Delete();
	virtual void DeleteContents();
//	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
//		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	void InsertPath( CListDirEntries &list, IsoDirectory &isoDir );
//	virtual CString GetFullPath( const CString &strDir );
//	virtual CString GetFullPathEx( const CString &strName, BOOL bExtract,
//		LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL );
	virtual void SetModifiedFlag( DOCPOS pos, BOOL bModified = TRUE );
//	virtual BOOL CopyFile( DOCPOS posDest, CDocDir *pDocDirSource, DOCPOS posSource, BOOL bPhysCopy,
//		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );

//	virtual int GetSubIcon() { return IconDir; }
//	virtual BOOL DoFileSave();
//	virtual void Invalidate() { }	// don't m_bScanned = FALSE;

//	virtual BOOL RemoveFileDir( DOCPOS pos );
//	void GetMarkedList( CStringArray &astrFiles, __int64 &nSize, BOOL bInclDirs=FALSE );

protected:
//	CDocDirIsoRoot *m_pIsoRoot;		// base document; i.e. of class CDocDirIsoRoot
//	CListDirEntries m_listIsoNew;

//	CString GetSubPath();
//	void AssureDirIsoNew();

private:
    static CStorage<CDocDirIsoSub> s_storeDocs;

};
