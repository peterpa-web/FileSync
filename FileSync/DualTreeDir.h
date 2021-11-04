#pragma once
#include "DualTree.h"
#include "ViewDirItem.h"

typedef CDualTree<CViewDirItem> CDualTreeDirBase;

class CDualTreeDirData;

class CDualTreeDir :
	public CDualTreeDirBase
{
public:
//	CDualTreeDir(void);
	CDualTreeDir(CDualTreeDirData &data);
	~CDualTreeDir(void);
	HTREEITEM InsertItem( POSITION pos ); // insert item into tree ctrl
	BOOL DeleteItem( HTREEITEM hItem ) { return CDualTreeDirBase::DeleteItem( hItem ); }
	BOOL DeleteItem( TREEPOS pos );
//	BOOL ExpandAll( HTREEITEM hItem );
	HTREEITEM GetFirstSel();
	HTREEITEM GetNextSel(HTREEITEM hItem);
//	BOOL IsAnySel(); // for current side 10200901 del
	BOOL AllSelAreReady(); // for current side
	void SelDiffChilds( int nSide, HTREEITEM hItem );
	virtual void ShowPopup( LPPOINT ppt );
	virtual void OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo);
	BOOL DeleteAllItems( );
	void UpdRealItem(TREEPOS &pos);
	void UpdColumnSize();
	TREEPOS GetFirstChildPos( TREEPOS posParent ) const;
	TREEPOS GetNextSiblingPos( TREEPOS pos ) const;
#ifdef _DEBUG
	const CString GetItemNameDebug( TREEPOS pos );
#endif
	void InvalidateItem( TREEPOS pos);
	BOOL HasDirIcon( TREEPOS pos ) const;
	BOOL IsArcExpand( TREEPOS pos ) const;
	BOOL Expand( TREEPOS pos );
	BOOL Collapse( HTREEITEM hItem ) { return CDualTreeDirBase::Expand( hItem, TVE_COLLAPSE ); }
//	BOOL Collapse( TREEPOS pos );
//	BOOL Free( TREEPOS pos );
	void FreeItemRecursive( TREEPOS pos, BOOL bDel=FALSE );
	BOOL IsExpanding() const { return m_bExpanding; }
	BOOL EnsureVisible( HTREEITEM hItem ) { return CDualTreeDirBase::EnsureVisible(hItem); }
	BOOL EnsureVisible( TREEPOS pos );
	TREEPOS GetItemParentPos( HTREEITEM hItem ) const;
	BOOL IsItemVisible( TREEPOS pos );
	BOOL LockWindowUpdate();
	void UnlockWindowUpdate();
	BOOL IsLockedWindowUpdate() const { return m_bLocked; }
	void SetBusyCursor( BOOL bBusy=TRUE );
	BOOL Optimize();
	void Link();
	int GetSelFiles( int nSide );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
	void SelectSingle(HTREEITEM hItem);
	void SelectToggle(HTREEITEM hItem);
	void SelectRange(HTREEITEM hItem);
	HTREEITEM GetFirstSelInt(HTREEITEM hItem);
	int GetFileCount( int nSide, HTREEITEM hItem );

	BOOL m_bDrag;
	BOOL m_bExpanding;
	BOOL m_bLocked;
	BOOL m_bBusyCursor;
	DWORD m_dwTicksOptimize;

private:
	CDualTreeDirData &m_data;

public:
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
