#pragma once


// CToolBarFixed

class CToolBarFixed : public CToolBar
{
	DECLARE_DYNAMIC(CToolBarFixed)

public:
	CToolBarFixed();
	virtual ~CToolBarFixed();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


