#pragma once

#include "DualListBox.h"
#include "ViewFileSync.h"

// CViewList view

class CDocFileSync;

class CViewList : public CViewFileSync
{
	friend class CUndoTaskFileSync;

	DECLARE_DYNAMIC(CViewList)

protected:
	CViewList();           // protected constructor used by dynamic creation
	virtual ~CViewList();

// Overrides
protected:
	CDualListBox m_list;

public:
	virtual void DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void OnListDblClk(UINT nItem, int nSide, const CRect &rect, CPoint point);
	virtual void OnListLButtonDown();
	virtual void OnListVScroll();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate2();
	virtual void MoveClient( LPCRECT lpRect );
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
};


