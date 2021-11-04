#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include "DocFileSync.h"
#include "ListDirEntries.h"
//#include "QueryZip.h"
#include "pidl.h"
#include "Storage.h"

class CDocDir :
    public CDocFileSync, protected CStorageObj
{
	DECLARE_DYNCREATE(CDocDir)

public:
	CDocDir(void);
	void Init( const CString &strName, CDocDir *pParent, DOCPOS posParent );
	virtual CDocDir* CreateSubDoc( const CString &strName, DOCPOS pos );
	virtual ~CDocDir(void);

	//--------------------------------

	class CMerge
	{
	public:
		CMerge( CListDirEntries &listL, CListDirEntries &listR );
		BOOL HasMore() { return m_posL != NULL || m_posR != NULL; }
		int CompareDoc();
		void GetNext( int nCompare );
		DOCPOS GetPosL() const { return m_posL; }
		DOCPOS GetPosR() const { return m_posR; }
		CDirEntry & GetDirEntryL() { return m_listL.GetAt(m_posL); }
		CDirEntry & GetDirEntryR() { return m_listR.GetAt(m_posR); }
		const CDirEntry & GetDirEntryL() const { return m_listL.GetAt(m_posL); }
		const CDirEntry & GetDirEntryR() const { return m_listR.GetAt(m_posR); }
#pragma warning(push)
#pragma warning(disable:4172)
		const CString & GetNameL() const { return ( m_posL == NULL ? NULL : GetDirEntryL().GetName() ); }
		const CString & GetNameR() const { return ( m_posR == NULL ? NULL : GetDirEntryR().GetName() ); }
#pragma warning(pop)
		void CopyR2L();
		BOOL IsChanged();

	protected:
		CListDirEntries & m_listL;
		CListDirEntries & m_listR;
		DOCPOS m_posL;
		DOCPOS m_posR;
	};

	//--------------------------------

	friend class CMerge;
	static BOOL CheckConn(const CString &strPath, BOOL bConn=FALSE);

	const CDirEntry & GetDirEntry( DOCPOS pos ) const { return m_list.GetAt( pos ); } // ?temp
	CDirEntry & GetDirEntry( DOCPOS pos ) { return m_list.GetAt( pos ); }
	const CDirEntry & GetMyDirEntry() const { ASSERT(GetParentDoc()!=NULL); return GetParentDoc()->GetDirEntry(m_posParent); }
	CDirEntry & GetMyDirEntry() { ASSERT(GetParentDoc()!=NULL); return GetParentDoc()->GetDirEntry(m_posParent); }
	DOCPOS FindDir( const CDocDir *pDir ) const { return m_list.Find(pDir); }
	CListDirEntries & GetList() { return m_list; }
	void UpdPathName();

	virtual void Delete() { ASSERT( FALSE ); } // to be overwritten
	virtual void DeleteContents();
	virtual BOOL ResetAll();
	virtual BOOL CheckPath() { return FALSE; }
//	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData);
	virtual BOOL Refresh( int nSide, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual int GetMaxLineLen() const { ASSERT(FALSE); return 0; } // dummy - not m_nMaxLineLen

	virtual CString GetFullPath( const CString &strDir );
	virtual CString GetFullPathLnk( const CString &strDir );
	virtual CString GetFullPathEx( BOOL bExtract );
	virtual CString GetFullPathEx( const CString &strDir, BOOL bExtract,
		LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL );
	virtual CString GetFullPathEx( DOCPOS pos, BOOL bExtract,
		LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL );
	virtual CDocFileSync* OpenDocumentFile( DOCPOS pos );
	virtual CDocFileSync* OpenDocumentFile( DOCPOS pos, CDocTemplFileSync* pTemplate,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual BOOL CreateDir();
	virtual BOOL CreateDir( DOCPOS pos );
	virtual BOOL CopyFile( DOCPOS posDest, CDocDir *pDocDirSource, DOCPOS posSource, BOOL bPhysCopy,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual void MakeRW( DOCPOS pos, BOOL b = TRUE );
	virtual void MakeRW( CDirEntry &de, BOOL b = TRUE );
	DOCPOS InsertDummy( BOOL bDir, const CString strName );
	void InsertFile( const CDirEntry &de );
	virtual BOOL RemoveFileDir( DOCPOS pos );

	virtual BOOL PreReScanAuto( int nSide, const CString &strBasePath );
	BOOL PreReScanAutoBase( int nSide, const CString &strBasePath, USHORT usMode );
	virtual BOOL ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual ULONG ComputeCRC( DOCPOS pos, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	ULONG CRC32T( ULONG crc, LPCSTR buf, size_t len, BOOL &bInComm);
	virtual ULONG ComputeCRCText( DOCPOS pos, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual int GetSubIcon();
	virtual BOOL RemoveTempRoot(void);
	virtual BOOL DoFileSave();
	virtual void SetModifiedFlag( BOOL bModified = TRUE );
	virtual void SetModifiedFlag( DOCPOS pos, BOOL bModified = TRUE );
	virtual BOOL SaveModified( int nSide );

	void SetPIDL(LPITEMIDLIST pidl) { m_pidl = pidl; }
	void ResetPIDL() { m_pidl.Empty(); }
	virtual LPITEMIDLIST GetPIDL() const { return m_pidl; }
	CString GetName() const { return m_strName; }
//	void SetBasePath(LPCTSTR lpszPathName) { m_strBasePath = lpszPathName; }
	virtual void SetBasePath(LPCTSTR lpszPathName) { ASSERT( FALSE ); }
	virtual void Invalidate() { m_bScanned = FALSE; }
	BOOL IsScanned() const { return m_bScanned; }
	BOOL HasAnyDocs() const;
	virtual BOOL IsRoot() const { return FALSE; }
	BOOL MarkDirAny();
	BOOL MarkDirAll();
	void SetExtr();
	void MarkFiles( DWORD maskIncl, DWORD maskExcl );

	static DWORD GetLastError() { return s_dwLastError; }
	static void ResetOverwriteNewer() { s_bAskOverwriteNewer = TRUE; }

#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:
//	int m_nMaxLineLen;		// ???
	CListDirEntries m_list;
	CString m_strName;	// path element name
	CPIDL m_pidl;	// for BrowseForFolders
	BOOL m_bScanned;
	static DWORD s_dwLastError;
	static BOOL s_bAskOverwriteNewer;
	static CString s_strPing;
	static BOOL s_bSkipComm;	// skip slash star marked comments in text files
};
