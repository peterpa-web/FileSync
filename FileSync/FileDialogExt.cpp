// FileDialogExt.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "FileDialogExt.h"


// CFileDialogExt

//IMPLEMENT_DYNAMIC(CFileDialogExt, CFileDialog)
CFileDialogExt::CFileDialogExt(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
{
}

CFileDialogExt::~CFileDialogExt()
{
}


BEGIN_MESSAGE_MAP(CFileDialogExt, CFileDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnSelChangeType)
END_MESSAGE_MAP()



// CFileDialogExt-Meldungshandler


BOOL CFileDialogExt::OnInitDialog()
{
	CFileDialog::OnInitDialog();

	// TODO:  hier zusätzliche Initialisierung hinzufügen.
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_TYPE);
	pCombo->AddString(_T("default"));
	pCombo->AddString(_T("hex"));
	pCombo->AddString(_T("text"));
	pCombo->AddString(_T("xml"));
	if ( m_strType.IsEmpty() )
		m_strType = _T("default");
	pCombo->SelectString(0, m_strType);
	return TRUE;  // return TRUE unless you set the focus to a control
	// AUSNAHME: OCX-Eigenschaftenseite muss FALSE zurückgeben.
}

void CFileDialogExt::OnSelChangeType()
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_TYPE);
	int n = pCombo->GetCurSel();
	pCombo->GetLBText(n, m_strType);
}

// Bugfix virtual fkt map VS2010 Beta2

BOOL CFileDialogExt::OnFileNameOK()
{
	// TODO: Add your specialized code here and/or call the base class

	return FALSE;
}


BOOL CFileDialogExt::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	ASSERT(pResult != NULL);
	OFNOTIFY* pNotify = (OFNOTIFY*)lParam;
	if (pNotify->hdr.code == CDN_FILEOK) {
		*pResult = OnFileNameOK();
		return TRUE;
	}

	return CFileDialog::OnNotify(wParam, lParam, pResult);
}
