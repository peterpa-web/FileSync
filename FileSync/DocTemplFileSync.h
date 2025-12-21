// MDI support (zero or more documents)

class CViewFileSync;
class CDocFileSync;
//class REBProgressManager;

class CDocTemplFileSync : public CDocTemplate
{
	DECLARE_DYNAMIC(CDocTemplFileSync)
#pragma once

// Constructors
public:
	CDocTemplFileSync(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

// Implementation
public:
	// Menu and accel table for MDI Child windows of this type
	HMENU m_hMenuShared;
	HACCEL m_hAccelTable;

	virtual ~CDocTemplFileSync();
	virtual void LoadTemplate();
	virtual Confidence MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch);
	virtual void AddDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
	virtual POSITION GetFirstDocPosition() const;
	virtual CDocument* GetNextDoc(POSITION& rPos) const;
	virtual CDocument* OpenDocumentFile( LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE );	// abstract VC 10
	virtual CDocument* OpenDocumentFile( LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible ); // abstract VC 10
	virtual CDocument* OpenDocumentFile( LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible, 
										 LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	virtual void SetDefaultTitle(CDocument* pDocument);
	virtual CFrameWndEx* CreateNewFrame(CDocument* pDoc, CFrameWndEx* pOther);

//	void AddExtension( const char *pszExt ) { m_astrExt.Add( pszExt ); }
//	CRuntimeClass* GetViewClass() { return m_pViewClass; }
	CViewFileSync* GetView() { return m_pView; }
	void SetView( CViewFileSync* pView ) { m_pView = pView; }
	UINT GetResourceID() const { return m_nIDResource; }
	void SetDefault( BOOL b = TRUE ) { m_bDefault = b; };
	void SetIconNo( int nIcon ) { m_nIcon = nIcon; }
	int GetIconNo() const { return m_nIcon; }
	void GetProfile( LPCTSTR pszSection, LPCSTR pszDefault );
	void WriteProfile() const;
	CStringArray & GetExtensionsArray() { return m_astrExt; };
	CString GetTypeString() const;
	BOOL IsDocKindOf( const CRuntimeClass* pClass ) const { return m_pDocClass->IsDerivedFrom(pClass); }
	BOOL IsViewKindOf( const CRuntimeClass* pClass ) const { return m_pViewClass->IsDerivedFrom(pClass); }

#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG

protected:  // standard implementation
	CPtrList m_docList;          // open documents of this type
//	UINT m_nUntitledCount;   // start at 0, for "Document1" title
	CStringArray m_astrExt;		// supported extensions
	CViewFileSync *m_pView;
	BOOL m_bDefault;
	CString m_strSection;		// from GetProfile for WriteProfile
	int m_nIcon;

	CDocFileSync* ReOpenDocumentFile( CDocFileSync* pDocument, LPCTSTR lpszPathName, BOOL bMakeVisible,
		LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
};

