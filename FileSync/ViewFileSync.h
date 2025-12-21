#pragma once

//#include "DualListBox.h"
//#include "DualTreeCtrl.h"
#include "UndoBuffer.h"
#include "afxtempl.h"
#include "ChangeNotification.h"
#include "ComboBoxdir.h"
#include "ToolButton.h"
#include "ListBoxMode.h"
#include "REBProgressDialog.h"

class CMainFrame;

DWORD CALLBACK CopyProgressRoutine(
	__in      LARGE_INTEGER TotalFileSize,
	__in      LARGE_INTEGER TotalBytesTransferred,
	__in      LARGE_INTEGER StreamSize,
	__in      LARGE_INTEGER StreamBytesTransferred,
	__in      DWORD dwStreamNumber,
	__in      DWORD dwCallbackReason,
	__in      HANDLE hSourceFile,
	__in      HANDLE hDestinationFile,
	__in_opt  LPVOID lpData
);

// CViewFileSync view

class CDocFileSync;

class CViewFileSync : public CView
{
	DECLARE_DYNAMIC(CViewFileSync)

protected:
	CViewFileSync();           // protected constructor used by dynamic creation
	virtual ~CViewFileSync();
	void RemoveListMode();

// Overrides
public:
	virtual void DeleteContents();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL CompareView() = 0;
	virtual BOOL OnRefresh();
	void OnFileMode(int nSide);
	UINT GetModeBitmap(int nSide, BOOL bYellow = FALSE);
	void OnViewChangeTo(UINT nIDResource);
	void Warn( int nSide ) { nSide==0 ? m_comboDirLeft.Warn( TRUE ) : m_comboDirRight.Warn( TRUE ); }
	void WarnReset( int nSide ) { nSide==0 ? m_comboDirLeft.Warn( FALSE ) : m_comboDirRight.Warn( FALSE ); }

	static COLORREF m_crBkColor;
	static COLORREF m_crWndColor;
	COLORREF m_crMark;
	COLORREF m_crMarkLite;
	COLORREF m_crMarkDir;
	COLORREF m_crTxtChanged;
	COLORREF m_crMarkSingle;

	CFont m_fontEdit;
	CFont m_fontList;
	CToolButton m_buttonsLeft[3];
	CToolButton m_buttonsRight[3];
	CComboBoxDir m_comboDirLeft;
	CComboBoxDir m_comboDirRight;
	CImageList m_imagelist;
	CUndoBuffer m_undoBuffer;
	CDocFileSync* m_pDoc[2];
	CChangeNotification changeNotify[2];
	BOOL m_bChanged[2];
	CBitmap m_bmFileOpen;
	CBitmap m_bmDirOpen;
	CBitmap m_bmDirComp;
	CBitmap m_bmFileComp;
	int m_nWidthLineNo;

public:
	virtual void ResetSide() {}
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view - DUMMY
	virtual void OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo);

	virtual BOOL Show( BOOL b );
	virtual BOOL OnIdle(LONG lCount);
	virtual void ChangeDirs( const CString &strPathLeft, const CString &strPathRight );
	const CString & GetLastFilePath() const { return m_strLastFilePath; }

	enum Side {
		left = 0,
		right = 1,
		common = 2		// only for painting
	};
	static Side GetSide() { return s_nSide; }
    static void SetSide( Side s ) { s_nSide = s; }
	CDocFileSync* GetDoc(int nSide) const { return m_pDoc[nSide]; }
	CDocFileSync* GetCurrDoc() const { return GetDoc(s_nSide); }
	CDocFileSync* GetOtherDoc() const { return GetDoc(1 - s_nSide); }
	virtual void SetDocument( CDocFileSync *pDocument, int nSide );
	void SetCurrDocument( CDocFileSync *pDocument ) { SetDocument( pDocument, s_nSide ); }
	void SetOtherDocument( CDocFileSync *pDocument ) { SetDocument( pDocument, 1 - s_nSide ); }
	void UpdateEditDir( int nSide, BOOL bStore = TRUE );
	void SaveHistory();
	int GetCurrToolbarID() { return m_nToolbarID[s_nSide]; }
	int GetMenueID() { return m_nMenueID; }
	int GetTitleID() { return m_nTitleID; }
	CMenu* GetMenu() { return &m_menu; }
	virtual BOOL IsNoDifference() { return FALSE; }
	virtual void CompareEnable( BOOL bEnable = TRUE );
	virtual void ChangedSelCombo( int nSide, BOOL bReset ); // from ComboBoxDir; common disallowed
	virtual void SelectSide( Side s ); // common disallowed
	virtual void SelectSide2( Side s ); // from ComboBoxDir; unsel only; common disallowed
	void SelectSide( int s ) { if (s == left) SelectSide(left); else if (s == right) SelectSide(right); } // ign. common
	CMainFrame* GetParentFrame();
	virtual void OnDeactivateApp();
	virtual BOOL GetMainIcon(HICON &hIcon, BOOL bBigIcon);
	void UpdateModeButtons();
	LPPROGRESS_ROUTINE GetProgressRoutine() { return &CopyProgressRoutine; }
	REBProgressManager* GetProgressMan() { return m_pProgressMan; }
	void StartProgress( __int64 nRange );

protected:
	static Side s_nSide;	// left or right
	static BOOL m_bCompareEnabled;
	CMenu m_menu;
	int m_nTitleID;
	int m_nMenueID;
	int m_nToolbarID[2];
	BOOL m_bSaveMore;
	CString m_strLastFilePath;
	CString m_strColorsSection;
	CString m_strOldPath[2];
	CListBoxMode* m_pListMode;
	REBProgressManager *m_pProgressMan;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void OnFileSave( Side s );
	BOOL SaveModified( Side s );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate();
	virtual void OnInitialUpdate2() = 0;
	virtual void MoveClient( LPCRECT lpRect ) = 0;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileOpenDirRight();
	afx_msg void OnFileOpenDirLeft();
	afx_msg void OnFileOpenRight();
	afx_msg void OnFileOpenLeft();
	afx_msg void OnUpdateViewRefresh(CCmdUI *pCmdUI);
	afx_msg void OnViewRefresh();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFileSaveLeft(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveLeft();
	afx_msg void OnUpdateFileSaveRight(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveRight();
	afx_msg void OnFileSaveLeftAs();
	afx_msg void OnFileSaveRightAs();
	afx_msg void OnUpdateViewDircomp(CCmdUI *pCmdUI);
	afx_msg void OnViewDircomp();
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileOpen();
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnViewAssociations();
	afx_msg void OnViewColors();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileModeleft();
	afx_msg void OnFileModeright();
	afx_msg void OnViewCheckleft();
	afx_msg void OnViewCheckright();
	afx_msg void OnUpdateFileModeleft(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileModeright(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewCheckleft(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewCheckright(CCmdUI *pCmdUI);
	afx_msg void OnListModeDblClk();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnViewText();
	afx_msg void OnHelpContents();
	afx_msg void OnViewXml();
	afx_msg void OnViewHex();
};


