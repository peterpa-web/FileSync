// DialogReplace.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "FileSync.h"
#include "DialogReplace.h"


// CDialogReplace-Dialogfeld

IMPLEMENT_DYNAMIC(CDialogReplace, CDialog)
CDialogReplace::CDialogReplace(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogReplace::IDD, pParent)
{
}

CDialogReplace::~CDialogReplace()
{
}

void CDialogReplace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogReplace, CDialog)
END_MESSAGE_MAP()


// CDialogReplace-Meldungshandler
BOOL CDialogReplace::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	HICON icon = LoadIcon( NULL, IDI_QUESTION );
	((CStatic*)GetDlgItem(IDC_BIGICON))->SetIcon(icon);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
