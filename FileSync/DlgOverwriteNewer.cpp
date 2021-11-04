// DlgOverwriteNewer.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "DlgOverwriteNewer.h"


// CColorsDlg dialog

IMPLEMENT_DYNAMIC(CDlgOverwriteNewer, CDialog)
CDlgOverwriteNewer::CDlgOverwriteNewer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgOverwriteNewer::IDD, pParent)
{
}

CDlgOverwriteNewer::~CDlgOverwriteNewer()
{
}

void CDlgOverwriteNewer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgOverwriteNewer, CDialog)
	ON_BN_CLICKED(IDRETRY, OnRetry)
END_MESSAGE_MAP()


// CColorsDlg message handlers

BOOL CDlgOverwriteNewer::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_STATIC_TEXT1)->SetWindowTextW( m_strFileName );
	// TODO: set default button to IDCANCEL and show Icon
	HICON icon = LoadIcon( NULL, IDI_EXCLAMATION );
	((CStatic*)GetDlgItem(IDC_BIGICON))->SetIcon(icon);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgOverwriteNewer::OnRetry()
{
	EndDialog(IDRETRY);
}

