#pragma once
#include "ComboSearch.h"
#include "ToolBarFixed.h"


// CToolBarSearch

class CToolBarSearch : public CToolBarFixed
{
	DECLARE_DYNAMIC(CToolBarSearch)

public:
	CToolBarSearch();
	virtual ~CToolBarSearch();

	BOOL CreateExtra();
	CString GetSearchText() { return m_comboSearch.GetSearchText(); }

protected:
	CFont gSmallFont;
	CComboSearch m_comboSearch;

protected:
	DECLARE_MESSAGE_MAP()
};


