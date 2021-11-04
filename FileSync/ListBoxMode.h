#pragma once


// CListBoxMode

class CListBoxMode : public CListBox
{
	DECLARE_DYNAMIC(CListBoxMode)

public:
	CListBoxMode();
	virtual ~CListBoxMode();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


