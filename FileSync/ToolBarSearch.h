#pragma once
#include "ComboSearch.h"
#include "ToolBarFixed.h"
#include "resource.h"


// CToolBarSearch

class CToolBarSearch : public CToolBarFixed
{
	DECLARE_DYNAMIC(CToolBarSearch)

public:
	CToolBarSearch();
	virtual ~CToolBarSearch();
	void Reset();

	BOOL CreateExtra();
	CString GetSearchText() { return L"?????"; }		// m_comboSearch.GetText();

protected:
//	CFont gSmallFont;
//	CComboSearch m_comboSearch;
	CMFCToolBarComboBoxButton m_comboSearch;

protected:
	DECLARE_MESSAGE_MAP()
};


