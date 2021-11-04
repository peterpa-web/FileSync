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


BEGIN_MESSAGE_MAP(CToolBarSearch, CToolBarFixed)
END_MESSAGE_MAP()



// CToolBarSearch-Meldungshandler

BOOL CToolBarSearch::CreateExtra()
{
	// set up small font
	gSmallFont.CreatePointFont(90, _T("DEFAULT"), NULL);

	//set up the ComboBox control as a snap mode select box
    //
    //First get the index of the placeholder's position in the toolbar
    int index = 0;
    while (GetItemID(index) != ID_SEARCH_C) 
		index++;

    //next convert that button to a seperator and get its position
    SetButtonInfo(index, ID_SEARCH_C, TBBS_SEPARATOR, 180);
	CRect rect;
    GetItemRect(index, &rect);

    //expand the rectangle to allow the combo box room to drop down
    rect.top+=2;
    rect.bottom += 200;

    // then .Create the combo box and show it
    if (!m_comboSearch.Create(WS_CHILD|WS_VISIBLE|CBS_AUTOHSCROLL| 
                                       CBS_DROPDOWN|CBS_HASSTRINGS,
                                       rect, this, IDC_SEARCH_C))
    {
        TRACE0("Failed to create combo-box\n");
        return FALSE;
    }
	m_comboSearch.SendMessage(WM_SETFONT, (WPARAM)HFONT(gSmallFont),TRUE);
	m_comboSearch.ShowWindow(SW_SHOW);
	return TRUE;
}
