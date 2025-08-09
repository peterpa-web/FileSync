#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "DocDir.h"
#include "DualTreeDir.h"
#include "DualTreeDirData.h"
//#include "REBProgressDialog.h"
#include "ThreadBack.h"
#include "TaskDirScan.h"

#include "ViewFileSync.h"

class CDocDir;

class CViewDir :
	public CViewFileSync
{
	friend class CUndoDir;

	DECLARE_DYNCREATE(CViewDir)

public:
	CViewDir(void);
	virtual ~CViewDir(void);

	virtual void ResetSide();
	virtual BOOL Show( BOOL b );
	virtual BOOL OnIdle(LONG lCount);
	BOOL ReactiveDocument(int nSide, CDocDir *pDoc, const CString& strBasePath);
	virtual void SetDocument( CDocFileSync *pDocument, int nSide );
	virtual void CompareEnable( BOOL bEnable = TRUE );
	virtual void ChangedSelCombo( int nSide, BOOL bReset ); // from ComboBoxDir; common disallowed
	virtual void SelectSide( Side s ); // common disallowed
	void SelectSide( int s ) { if (s == left) SelectSide(left); else if (s == right) SelectSide(right); } // ign. common
	virtual void SelectSide2( Side s ); // from ComboBoxDir; unsel only; common disallowed
	virtual BOOL GetMainIcon(HICON &hIcon, BOOL bBigIcon);

protected:
	int m_nWidthFileSize;
	int m_nCharOffs;
	BOOL m_bUndoEnabled;
	CDualTreeDirData m_treeData;
	CDualTreeDir m_tree;  // ( m_treeData );
	CThreadBack m_threadBack;
//	REBProgressManager m_progressMan;

public:
	CDocDir* GetDoc(int nSide) const { return (CDocDir *)m_pDoc[nSide]; }
	CDocDir* GetDocL() const { return (CDocDir *)m_pDoc[0]; }
	CDocDir* GetDocR() const { return (CDocDir *)m_pDoc[1]; }
	virtual void DeleteContents();
	void MarkSelEqual( BOOL b );

protected:
	void InitDocs();
	void InitDoc( const CString &strPath, int nSide );
//	void CompareItem( CDocDir::CMerge &merge, CViewDirItem &d, int nComp );
//	void CompareItem( int nSide, CViewDirItem &d, CDocDir *pDoc, POSITION pos );
//	void AddItem( const CViewDirItem &d, HTREEITEM hParent = TVI_ROOT );
	void ClearItemRecursive( HTREEITEM hItem, int nSide );
	BOOL RemoveItemRecursive( HTREEITEM hItem, int nSide );
	__int64 CopyItemRecursivePre( HTREEITEM hItem, int nSide );
	BOOL CopyItemRecursive( HTREEITEM hItem, int nSide, BOOL bPhysCopy );
	BOOL SaveModifItemRecursive( HTREEITEM hItem, int nSide, BOOL bRemoveExpanded );
	BOOL FindRW( HTREEITEM hItem, int nSide, BOOL &bRW );
	BOOL SetRWRecursive( HTREEITEM hItem, int nSide, BOOL bRW );
	void DelZipContents( HTREEITEM hItem );
	virtual BOOL CompareView();
	void ReCompareSide( int nSide, HTREEITEM hParentItem );		// 20060302
	virtual BOOL OnRefresh();
	void CreateSubItems( HTREEITEM hItem, TREEPOS posTree, CViewDirItem &d );
	void UpdateParentMark( HTREEITEM hItem );
	HTREEITEM SearchF( HTREEITEM hItem, const CString &strSearch );
//	void IdleRequest( int nIdleType ) {
//		m_nIdleTypeNew = nIdleType;
	//	TRACE1("CViewDir::IdleRequest(%d)\n", nIdleType);
//	}
	void OnFileOpen(int nSide);
	virtual void OnFileSave( Side s );

	void UpdateHScroll();
	HGLOBAL SelCopy();
	CString GetLastErrorText(DWORD dwLastError=0);

private:
//	HTREEITEM m_hIdleItem;
//	int m_nIdleType;
//	int m_nIdleTypeNew;
	enum IdleType {
		UpdateNone,
		UpdateIcons,
		UpdateDirs,
		UpdateAllDirs,
		CompareFiles,
		CompareDirs,
		UpdateAllIcons,
		CompareAllFiles,
		CompareAllDirs,
		RefreshSide
	};
//	int m_nIdleItems;
	CUndoDir *m_pTask;
	CMenu m_menuContext;
	CMenu m_menuContextDir;
	void OnTreeDblClk(HTREEITEM hItem, int nSide, CDocTemplFileSync* pTemplate);
	void OnType( UINT nIDResource );
	BOOL m_bSingleLine;
//	DWORD m_dwLastIdle;
	BOOL m_bInvalidate;
	int m_nSortType;

public:
	void OnTreeDblClk(HTREEITEM hItem, int nSide);
	void OnTreeItemexpanding(LPNMTREEVIEW pNMTreeView);

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate2();
	virtual void MoveClient( LPCRECT lpRect );
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	BOOL CheckUpdateCmd(CCmdUI *pCmdUI);

public:
	afx_msg void OnUpdateCmd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewNextDiff(CCmdUI *pCmdUI);
	afx_msg void OnViewNextDiff();
	afx_msg void OnUpdateViewPrevDiff(CCmdUI *pCmdUI);
	afx_msg void OnViewPrevDiff();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditReplacesel(CCmdUI *pCmdUI);
	afx_msg void OnEditReplacesel();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnFileOpenLeft();
	afx_msg void OnFileOpenRight();
	afx_msg void OnFileOpenDirLeft();
	afx_msg void OnFileOpenDirRight();
	afx_msg void OnUpdateViewIncludesubdirs(CCmdUI *pCmdUI);
	afx_msg void OnViewIncludesubdirs();
	afx_msg void OnUpdateViewExplorer(CCmdUI *pCmdUI);
	afx_msg void OnViewExplorer();
	afx_msg void OnUpdateViewFilesync(CCmdUI *pCmdUI);
	afx_msg void OnViewFilesync();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateEditPrev(CCmdUI *pCmdUI);
	afx_msg void OnEditPrev();
	afx_msg void OnUpdateEditNext(CCmdUI *pCmdUI);
	afx_msg void OnEditNext();
	afx_msg void OnTypeHex();
	afx_msg void OnTypeText();
	afx_msg void OnTypeXml();
	afx_msg void OnUpdateTypeRW(CCmdUI *pCmdUI);
	afx_msg void OnTypeRW();
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnViewColors();
//	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnUpdateTypeOpen(CCmdUI *pCmdUI);
	afx_msg void OnTypeOpen();
	afx_msg void OnDirCollapse();
	afx_msg void OnUpdateDirFree(CCmdUI *pCmdUI);
	afx_msg void OnDirFree();
	afx_msg void OnDirExpandpartial();
	afx_msg void OnDirSetequal();
	afx_msg void OnUpdateDirExpandpartial(CCmdUI *pCmdUI);
	afx_msg void OnSearch();
	afx_msg void OnSearchB();
	afx_msg void OnSearchF();
	afx_msg void OnUpdateDirSelectdiffs(CCmdUI *pCmdUI);
	afx_msg void OnDirSelectdiffs();
	afx_msg void OnViewAssociations();
	afx_msg void OnHelpContext();
//	afx_msg LRESULT OnMsgUpdTree(UINT wParam, LONG lParam);
	afx_msg void OnUpdateSortName(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortMKS(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortLeftSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortLeftSizeDesc(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortLeftTime(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortLeftTimeDesc(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortRightSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortRightSizeDesc(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortRightTime(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSortRightTimeDesc(CCmdUI *pCmdUI);
	afx_msg void OnSortName();
	afx_msg void OnSortMKS();
	afx_msg void OnSortLeftSize();
	afx_msg void OnSortLeftSizeDesc();
	afx_msg void OnSortLeftTime();
	afx_msg void OnSortLeftTimeDesc();
	afx_msg void OnSortRightSize();
	afx_msg void OnSortRightSizeDesc();
	afx_msg void OnSortRightTime();
	afx_msg void OnSortRightTimeDesc();
	afx_msg LRESULT OnUserStartIdle(UINT wParam, LONG lParam);
	afx_msg void OnUpdateViewProject(CCmdUI *pCmdUI);
	afx_msg void OnViewProject();
	afx_msg void OnUpdateViewLink(CCmdUI *pCmdUI);
	afx_msg void OnViewLink();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg LRESULT OnUserErr(UINT wParam, LONG lParam);
};
