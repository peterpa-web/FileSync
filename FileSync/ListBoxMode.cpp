// ListBoxMode.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "BitmapTransparent.h"
#include "afxpriv.h"
#include "ListBoxMode.h"


// CListBoxMode

IMPLEMENT_DYNAMIC(CListBoxMode, CListBox)

CListBoxMode::CListBoxMode()
{

}

CListBoxMode::~CListBoxMode()
{
}


BEGIN_MESSAGE_MAP(CListBoxMode, CListBox)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CListBoxMode message handlers




void CListBoxMode::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);
	ReleaseCapture();
	ShowWindow(SW_HIDE);
}


BOOL CListBoxMode::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
    {
        case WM_LBUTTONUP:
			{
				NMHDR nmh;
				nmh.code = LBN_DBLCLK;    // Message type defined by control.
				nmh.idFrom = GetDlgCtrlID();
				nmh.hwndFrom = m_hWnd;
				GetParent()->SendMessage( WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh );	
				return TRUE;
			}
			break;
		case WM_KEYDOWN:
        	int nVirtKey = (int) pMsg->wParam;
        	if (nVirtKey == VK_ESCAPE)
				ShowWindow(SW_HIDE);
        	else if (nVirtKey == VK_RETURN) {
				NMHDR nmh;
				nmh.code = LBN_DBLCLK;    // Message type defined by control.
				nmh.idFrom = GetDlgCtrlID();
				nmh.hwndFrom = m_hWnd;
				GetParent()->SendMessage( WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh );	
				return TRUE;
			}
	}
	return CListBox::PreTranslateMessage(pMsg);
}


void CListBoxMode::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = 17; // bitmaps: 19;
}


void CListBoxMode::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	UINT nRes = lpDrawItemStruct->itemData;
	TCHAR szFullText[256];
	CString strText;

	AfxLoadString(nRes, szFullText);
	AfxExtractSubString(strText, szFullText, 1, '\n');

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

   // Save these value to restore them when done drawing.
   COLORREF crOldTextColor = dc.GetTextColor();
   COLORREF crOldBkColor = dc.GetBkColor();

   // If this item is selected, set the background color 
   // and the text color to appropriate values. Also, erase
   // rect by filling it with the background color.
   COLORREF crNewBkColor = crOldBkColor;
   if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
      (lpDrawItemStruct->itemState & ODS_SELECTED))
   {
      dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	  crNewBkColor = ::GetSysColor(COLOR_HIGHLIGHT);
      dc.SetBkColor(crNewBkColor);
      dc.FillSolidRect(&lpDrawItemStruct->rcItem, 
         ::GetSysColor(COLOR_HIGHLIGHT));
   }
   else
      dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);

   	// Draw the bitmap
   	// CBitmap bmRaw;
   	CBitmapTransparent bmTr;
	if (bmTr.LoadBitmap(MAKEINTRESOURCE(nRes))) {
		bmTr.DrawTransparent(&dc, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
			RGB(0, 128, 128), 1);
	}

    // Draw the text.
    CRect rectText = lpDrawItemStruct->rcItem;
    rectText.left += 19;
    dc.DrawText(
		strText,
		strText.GetLength(),
		rectText,
		DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

    // Reset the background color and the text color back to their
    // original values.
    dc.SetTextColor(crOldTextColor);
    dc.SetBkColor(crOldBkColor);

    dc.Detach();
}
