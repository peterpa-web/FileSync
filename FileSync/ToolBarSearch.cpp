// ToolBarSearch.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "ToolBarSearch.h"


// CToolBarSearch

IMPLEMENT_DYNAMIC(CToolBarSearch, CToolBarFixed)
CToolBarSearch::CToolBarSearch() : m_comboSearch(ID_SEARCH_C, 2)
{
}

CToolBarSearch::~CToolBarSearch()
{
}

void CToolBarSearch::Reset()
{
//	CMFCToolBarComboBoxButton btncombo(ID_SEARCH_C, GetCmdMgr()->GetCmdImage(ID_EDIT_FIND));
	ReplaceButton(ID_SEARCH_C, m_comboSearch);
	m_comboSearch.EnableWindow(true);
	m_comboSearch.SetCenterVert();
	m_comboSearch.SetDropDownHeight(25);
	m_comboSearch.SetFlatMode();
	m_comboSearch.SetText(_T("this is a combo box"));
}


BEGIN_MESSAGE_MAP(CToolBarSearch, CToolBarFixed)
END_MESSAGE_MAP()



// CToolBarSearch-Meldungshandler

BOOL CToolBarSearch::CreateExtra()
{
	/*
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	// 10 point height Courier New font
	CDC* pDC = GetDC();
	int	cyPerInch = pDC->GetDeviceCaps(LOGPIXELSY);
	ReleaseDC(pDC);
	logfont.lfHeight = -MulDiv(8, cyPerInch, 72);
	logfont.lfWeight = FW_NORMAL;
	logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	static TCHAR BASED_CODE szFaceName2[] = _T("Arial");
	lstrcpy(logfont.lfFaceName, szFaceName2);
	gSmallFont.CreateFontIndirect(&logfont);
	*/
	// set up small font
//	gSmallFont.CreatePointFont(90, _T("DEFAULT"), NULL);

	//set up the ComboBox control as a snap mode select box
    //
    //First get the index of the placeholder's position in the toolbar
//    int index = 0;
//    while (GetItemID(index) != ID_SEARCH_C) 
//		index++;

    //next convert that button to a seperator and get its position
	// CObList listButtons
// POSITION posCombo
//    SetButtonInfo(index, ID_SEARCH_C, TBBS_SEPARATOR, 180);
//	CRect rect;
//    GetItemRect(index, &rect);

    //expand the rectangle to allow the combo box room to drop down
 //    rect.top += 2;
//    rect.top -= 2;
//	int h = rect.bottom;
//    rect.bottom += 200;

    // then .Create the combo box and show it
//    if (!m_comboSearch.Create(WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | WS_VSCROLL |
//                                       CBS_DROPDOWN | CBS_HASSTRINGS,
//                                       rect, this, IDC_SEARCH_C))
//    {
//        TRACE0("Failed to create combo-box\n");
//        return FALSE;
//    }
//	m_comboSearch.SendMessage(WM_SETFONT, (WPARAM)HFONT(gSmallFont),TRUE);
//	m_comboSearch.ShowWindow(SW_SHOW);
//	m_comboSearch.SetItemHeight(-1, h);
	return TRUE;
}
