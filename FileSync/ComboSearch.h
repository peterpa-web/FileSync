#pragma once


// CComboSearch

class CComboSearch : public CComboBox
{
	DECLARE_DYNAMIC(CComboSearch)

public:
	CComboSearch();
	virtual ~CComboSearch();

	CString GetSearchText();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelendok();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


