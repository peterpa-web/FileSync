// ColorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "ColorsDlg.h"


// CColorsDlg dialog

IMPLEMENT_DYNAMIC(CColorsDlg, CDialog)
CColorsDlg::CColorsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorsDlg::IDD, pParent)
{
	m_acrDef[0] = RGB( 127, 255, 255 );
	m_acrDef[1] = RGB( 255, 255, 180 );
	m_acrDef[2] = RGB( 255, 255, 127 );
	m_acrDef[3] = RGB( 220, 0, 0 );
	m_acrDef[4] = RGB( 140, 255, 128 );
}

CColorsDlg::~CColorsDlg()
{
}

void CColorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CColorsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_C1, OnBnClickedButtonC1)
	ON_BN_CLICKED(IDC_BUTTON_C2, OnBnClickedButtonC2)
	ON_BN_CLICKED(IDC_BUTTON_C3, OnBnClickedButtonC3)
	ON_BN_CLICKED(IDC_BUTTON_C4, OnBnClickedButtonC4)
	ON_BN_CLICKED(IDC_BUTTON_C5, OnBnClickedButtonC5)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_DEF, OnBnClickedButtonDef)
END_MESSAGE_MAP()


// CColorsDlg message handlers

BOOL CColorsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CColorsDlg::OnBnClickedButtonC1()
{
	OnBnClickedButtonCx( 0 );
}

void CColorsDlg::OnBnClickedButtonC2()
{
	OnBnClickedButtonCx( 1 );
}

void CColorsDlg::OnBnClickedButtonC3()
{
	OnBnClickedButtonCx( 2 );
}

void CColorsDlg::OnBnClickedButtonC4()
{
	OnBnClickedButtonCx( 3 );
}

void CColorsDlg::OnBnClickedButtonC5()
{
	OnBnClickedButtonCx( 4 );
}

void CColorsDlg::OnBnClickedButtonCx( int nCtrl )
{
	CColorDialog dlg( m_acr[nCtrl], CC_FULLOPEN, this );
	if ( dlg.DoModal() == IDOK )
	{
		m_acr[nCtrl] = dlg.GetColor();
		Invalidate();
	}
}

void CColorsDlg::OnOK()
{
	WriteProfile();

	CDialog::OnOK();
}

void CColorsDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT( nIDCtl >= IDC_BUTTON_C1 && nIDCtl < (IDC_BUTTON_C1 + 5) );

	UINT uStyle = DFCS_BUTTONPUSH;

	// This code only works with buttons.
	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

	CRect rectBk = lpDrawItemStruct->rcItem;
	CRect rectTxt = lpDrawItemStruct->rcItem;
	rectBk.DeflateRect( 2, 2 );

	// If drawing selected, add the pushed style to DrawFrameControl.
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		uStyle |= DFCS_PUSHED;
		rectTxt.OffsetRect( 1, 1 );
	}

	// Draw the button frame.
	::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
		DFC_BUTTON, uStyle);

	// Get the button's text.
	CString strText;
	GetDlgItem(nIDCtl)->GetWindowText(strText);

	// Draw the button text.
	if ( nIDCtl == IDC_BUTTON_C4 )
	{
		COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, m_acr[nIDCtl-IDC_BUTTON_C1]);
		::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
			rectTxt, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
	}
	else
	{
		COLORREF crOldBkColor = ::SetBkColor(lpDrawItemStruct->hDC, m_acr[nIDCtl-IDC_BUTTON_C1]);
		::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, rectBk, NULL, 0, NULL);

		::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
			rectTxt, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		::SetBkColor(lpDrawItemStruct->hDC, crOldBkColor);
	}

	if (lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		CRect rect = (lpDrawItemStruct->rcItem);
		rect.DeflateRect( 4, 4 );
		::DrawFocusRect( lpDrawItemStruct->hDC, rect );
	}
//	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CColorsDlg::GetProfile( LPCTSTR pszSection )
{
	m_strSection = pszSection;
	ASSERT( !m_strSection.IsEmpty() );
	ASSERT( m_strSection.GetLength() < 19 );
	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("COLORS") );
	if ( hSecKey == NULL )
	{
		for ( int n = 0; n < 5; ++n )
		{
			m_acr[n] = m_acrDef[n];
		}
		return;
	}

	for ( int n = 0; n < 5; ++n )
	{
		TCHAR szEntry[20];
		wsprintf( szEntry, _T("%s%d"), pszSection, n+1 );
		DWORD dwValue;
		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx(hSecKey, szEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lResult != ERROR_SUCCESS)
		{
			m_acr[n] = m_acrDef[n];
			continue;
		}

		ASSERT(dwType == REG_DWORD);
		ASSERT(dwCount == sizeof(DWORD));
		lResult = RegQueryValueEx(hSecKey, szEntry, NULL, &dwType,
			(LPBYTE)&dwValue, &dwCount);

		m_acr[n] = dwValue;
	}
	RegCloseKey( hSecKey );
}

void CColorsDlg::WriteProfile() const
{
	ASSERT( !m_strSection.IsEmpty() );

	HKEY hSecKey = AfxGetApp()->GetSectionKey( _T("COLORS") );
	ASSERT( hSecKey != NULL );
	if (hSecKey == NULL)
		return;
	for ( int n = 0; n < 5; ++n )
	{
		TCHAR szEntry[20];
		wsprintf( szEntry, _T("%s%d"), (LPCTSTR)m_strSection, n+1 );
		LONG lResult = RegSetValueEx(hSecKey, szEntry, NULL, REG_DWORD,
			(LPBYTE)&m_acr[n], sizeof(DWORD));
	}
	RegCloseKey(hSecKey);
}

void CColorsDlg::OnBnClickedButtonDef()
{
	for ( int n = 0; n < 5; ++n )
	{
		m_acr[n] = m_acrDef[n];
	}
	Invalidate();
}
