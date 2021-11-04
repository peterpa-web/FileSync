#pragma once
#include "afxtempl.h"
#include "DocDirArchive.h"

typedef struct _zip_file_entry {
	CString strName;
	DWORD dwSize;
	CTime dt;
	ULONG crc;
} CZipFileEntry;

class CDocDirZipRoot;

class CDocDirZipSub :
	public CDocDirArchive
{
	DECLARE_DYNCREATE(CDocDirZipSub)

public:
	CDocDirZipSub(void);
    virtual CDocDir* CreateSubDoc( const CString &strName, DOCPOS pos );
	~CDocDirZipSub(void);

    CDocDirZipSub & operator =(const CDocDirZipSub &src) { ASSERT(FALSE); return *this; }
	virtual void Delete();
	virtual void DeleteContents();
//	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
//		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	void InsertPath( CListDirEntries &list, CString strSubPath, const CZipFileEntry fe );
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
//    CDocDirZipRoot *m_pZipRoot;     // base document; i.e. of class CDocDirZipRoot
//    CListDirEntries m_listZipNew;

//	CString GetSubPath();
//    void AssureDirZipNew();

private:
    static CStorage<CDocDirZipSub> s_storeDocs;

};
