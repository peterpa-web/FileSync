#pragma once

#include "DualTreeItem.h"

typedef POSITION TREEPOS; // see DualTreeData.h

// CDualTreeCtrl

class CDualTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDualTreeCtrl)

public:
	CDualTreeCtrl();
	virtual ~CDualTreeCtrl();

	void PrepareFont( CFont *pFont ) { m_pFont = pFont; }
	COLORREF GetWndColor() { return m_crWnd; }
	void SetWndColor( COLORREF cr ) { m_crWnd = cr; }
	COLORREF GetMarkDirColor() { return m_crMarkDir; }
	void SetMarkDirColor( COLORREF cr ) { m_crMarkDir = cr; }
	COLORREF GetMarkDirColor2() { return m_crMarkDir2; }
	void SetMarkDirColor2( COLORREF cr ) { m_crMarkDir2 = cr; }
	COLORREF GetMarkLiteColor() { return m_crMarkLite; }
	void SetMarkLiteColor( COLORREF cr ) { m_crMarkLite = cr; }
	COLORREF GetTxtChangedColor() { return m_crTxtChanged; }
	void SetTxtChangedColor( COLORREF cr ) { m_crTxtChanged = cr; }
	COLORREF GetMarkSingleColor() { return m_crMarkSingle; }
	void SetMarkSingleColor( COLORREF cr ) { m_crMarkSingle = cr; }
	int GetSide() { return m_nSide; }
	void SetSide( CDualTreeItem::Side s ) { m_nSide = s; }
	int GetItemHeight() const { return m_nItemHeight; }
	int GetViewLineLen( int nResPixels ) const;
	int GetCharWidth() const { return m_nCharWidth; }
	void UpdCharsLeft( int nChars );
	void UpdCharsRight( int nChars );
	int GetOffsLeft() const { return m_nOffsLeft; }
	int GetOffsRight() const { return m_nOffsRight; }
	void EnableDraw( BOOL b ) { m_bEnableDraw = b; }
	BOOL IsEnabledDraw() { return m_bEnableDraw; }
	void EnableClick( BOOL b ) { m_bEnableClick = b; }
	HTREEITEM GetBottomItem(void);
	BOOL IsItemVisible( HTREEITEM hItem );
	void SetMenuContext( CMenu *pMenu ) { m_pMenuContext = pMenu; }
	void SetMenuContextDir( CMenu *pMenu ) { m_pMenuContextDir = pMenu; }
	HTREEITEM GetItemCurr() const { return m_hItemCurr; }
	CDualTreeItem::Side GetClickSide() const { return m_nClickSide; }
	TREEPOS GetItemPos( HTREEITEM hItem ) const { return (TREEPOS)GetItemData( hItem ); }
//	UINT GetMouseKeyFlags() const { return m_nMouseKeyFlags; } // // for notify NM_CLICK
	void InvalidateItem( HTREEITEM hItem );
	void AdjustVScroll();

protected:
	DECLARE_MESSAGE_MAP()

	CFont *m_pFont = nullptr;
	COLORREF m_crWnd = 0;
	COLORREF m_crMarkDir = 0;
	COLORREF m_crMarkDir2 = 0;
	COLORREF m_crMarkLite = 0;
	COLORREF m_crTxtChanged = 0;
	COLORREF m_crMarkSingle = 0;
	CDualTreeItem::Side m_nSide = CDualTreeItem::Side::left;
	int m_nItemHeight = 10;
	int m_nCharWidth = 5;
	int m_nCharsLeft = 0;
	int m_nCharsRight = 0;
	int m_nOffsLeft = 0;
	int m_nOffsRight = 200;
	BOOL m_bEnableDraw = TRUE;
	BOOL m_bEnableClick = TRUE;
	CMenu *m_pMenuContext = nullptr;
	CMenu *m_pMenuContextDir = nullptr;
	HTREEITEM m_hItemCurr = NULL;
	CDualTreeItem::Side m_nClickSide = CDualTreeItem::Side::common;
//	UINT m_nMouseKeyFlags = 0;	// for OnLButtonDown - notify NM_CLICK
	
public:
	void UpdOffs();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPNMTVCUSTOMDRAW lplvcd);
	virtual void DrawTreeItem(CDC* pDC, LPNMTVCUSTOMDRAW lplvcd) = 0;
	virtual void ShowPopup( LPPOINT ppt ) = 0;
	virtual void OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo) = 0;
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


