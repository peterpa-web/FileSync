// ToolBarSearch.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "ToolBarSearch.h"


// CToolBarSearch

IMPLEMENT_DYNAMIC(CToolBarSearch, CToolBarFixed)
CToolBarSearch::CToolBarSearch()
{
}

CToolBarSearch::~CToolBarSearch()
{
}

void CToolBarSearch::Reset()
{
	ReplaceButton(IDC_SEARCH_C, CMFCToolBarComboBoxButton(IDC_SEARCH_C, IDX_COMBO, CBS_DROPDOWN));
}


BEGIN_MESSAGE_MAP(CToolBarSearch, CToolBarFixed)
END_MESSAGE_MAP()



// CToolBarSearch-Meldungshandler

