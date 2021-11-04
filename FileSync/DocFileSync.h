#pragma once

typedef POSITION DOCPOS;

// CDocFileSync document

class CDocDir;
//class REBProgressManager;
class CDocTemplFileSync;

class CDocFileSync : public CDocument
{
	DECLARE_DYNAMIC(CDocFileSync)

public:
	CDocFileSync();
	virtual ~CDocFileSync();

	virtual BOOL ResetAll();
	virtual void DeleteContents();
#ifdef _DEBUG
	CDocDir * GetParentDoc() const;
#else
	CDocDir * GetParentDoc() const { return m_pParent; }
#endif
	void SetParent( CDocDir *pParent ) { m_pParent = pParent; }
	DOCPOS GetParentPos() const { return m_posParent; }
	void SetParentPos( POSITION pos ) { m_posParent = pos; }
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU=TRUE );
	virtual void Serialize(CArchive& ar) = 0;
	virtual int GetMaxLineLen() const = 0;
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData) = 0;
	virtual void SetParentAndPos( CDocDir *pParent, POSITION pos ) { m_pParent = pParent; m_posParent = pos; }
	virtual BOOL Refresh( int nSide, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData ) = 0;
	virtual void RefreshAttr();
	virtual __int64 GetFileSize();
	virtual BOOL IsReadOnlyFile(); // from directory

	virtual BOOL DoFileSave();
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE );
	virtual LPITEMIDLIST GetPIDL() const { return NULL; }
	virtual BOOL SaveModified( int nSide );

	CString GetBasePathName();
	CString GetBasePathName( CString strPath );

	CString GetPathNameView() const;

	BOOL IsReadOnly() const { return m_bReadOnly; }
	void SetReadOnly( BOOL b );	// local
	void SetReadOnlyFile( BOOL b ); // directory
	CDocTemplFileSync* GetDocTemplate() const;
	BOOL IsNoTemp() const;

	enum Icons {
		IconUnknown = -1,
		IconDir = 1,
		IconHex,
		IconZipRoot,
		IconText,
		IconXml,
		IconIsoRoot
	};

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	BOOL m_bLock;	// is in use
#define SET_DOC_LOCK(b) m_bLock=b
#else
#define SET_DOC_LOCK(b)
#endif

protected:
	CDocDir *m_pParent;		// parent document for set modified etc.
	DOCPOS m_posParent;	// position of dir entry in list of parent document
	BOOL m_bReadOnly;

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnCloseDocument();
	virtual void OnChangedViewList();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
};
