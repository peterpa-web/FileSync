#pragma once

class CDualTreeCtrl;

class CDualTreeItem
{
public:
	CDualTreeItem();

	enum Side {
		left   = 0,
		right  = 1,
		common = 2		// only for painting
	};
//	enum Expand {
//		expandAll  = 1,
//		expandPart = 2
//	};

	int GetIcon() const { return m_nIcon; }
	void SetIcon( int n ) { m_nIcon = n; }
//	const CString & GetName() const;
	virtual void DrawTreeItemSide(CDC* pDC, CDualTreeCtrl &tree, HTREEITEM hItem, LPNMTVCUSTOMDRAW lplvcd, int nSide) = 0;
	virtual void OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo) = 0;
	BOOL IsAnySel() const { return m_bChildSel | m_bSel; } // 20100118
	BOOL IsChildSel() const { return m_bChildSel; }
	BOOL IsSel() const { return m_bSel; }
	void Sel( BOOL b ) { m_bSel = b; }
	void SetChildSel( BOOL b ) { m_bChildSel = b; }
//	BOOL IsExpandAll() const { return (m_nExpand & expandAll); }
//	void SetExpandAll( BOOL b ) { m_nExpand = b ? m_nExpand | expandAll : m_nExpand & ~expandAll; }
//	BOOL IsExpandPart() const { return (m_nExpand & expandPart); }
//	void SetExpandPart( BOOL b ) { m_nExpand = b ? m_nExpand | expandPart : m_nExpand & ~expandPart; }

protected:
	int m_nIcon;
	BOOL m_bSel;
	BOOL m_bChildSel; // any child is sel
//	int m_nExpand;
};
