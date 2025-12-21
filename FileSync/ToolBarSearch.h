#pragma once
#include "ToolBarFixed.h"

#define IDX_COMBO 2

// CToolBarSearch

class CToolBarSearch : public CToolBarFixed
{
	DECLARE_DYNAMIC(CToolBarSearch)

public:
	CToolBarSearch();
	virtual ~CToolBarSearch();
	void Reset();

	CMFCToolBarComboBoxButton* GetCombo() { return  DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, GetButton(IDX_COMBO)); }
	LPCTSTR GetSearchText() { return GetCombo()->GetText(); }

protected:
	DECLARE_MESSAGE_MAP()
};


