// PrefDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileSync.h"
#include "DocText.h"

#include "PrefDlg.h"


// CPrefDlg dialog

IMPLEMENT_DYNAMIC(CPrefDlg, CDialog)
CPrefDlg::CPrefDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrefDlg::IDD, pParent)
	, m_nTabSize(0)
	, m_bIgnSpaces(FALSE)
	, m_bUnixLeft(FALSE)
	, m_bUnixRight(FALSE)
	, m_bReadOnlyLeft(FALSE)
	, m_bReadOnlyRight(FALSE)
	, m_strEncodingL(_T(""))
	, m_strEncodingR(_T(""))
{
}

CPrefDlg::~CPrefDlg()
{
}

void CPrefDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TABSIZE, m_nTabSize);
	DDV_MinMaxInt(pDX, m_nTabSize, 1, 10);
	DDX_Check(pDX, IDC_CHECK_IGN_SPACES, m_bIgnSpaces);
	DDX_Check(pDX, IDC_CHECK_UNIX_LEFT, m_bUnixLeft);
	DDX_Check(pDX, IDC_CHECK_UNIX_RIGHT, m_bUnixRight);
	DDX_Check(pDX, IDC_CHECK_RO_LEFT, m_bReadOnlyLeft);
	DDX_Check(pDX, IDC_CHECK_RO_RIGHT, m_bReadOnlyRight);
	DDX_Control(pDX, IDC_COMBO_CSET_L, m_comboCsetL);
	DDX_Control(pDX, IDC_COMBO_CSET_R, m_comboCsetR);
	DDX_CBString(pDX, IDC_COMBO_CSET_L, m_strEncodingL);
	DDX_CBString(pDX, IDC_COMBO_CSET_R, m_strEncodingR);
	DDX_Check(pDX, IDC_CHECK_FRO_LEFT, m_bFileROLeft);
	DDX_Check(pDX, IDC_CHECK_FRO_RIGHT, m_bFileRORight);
}


BEGIN_MESSAGE_MAP(CPrefDlg, CDialog)
END_MESSAGE_MAP()


// CPrefDlg message handlers

BOOL CPrefDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CSpinButtonCtrl *)GetDlgItem( IDC_SPIN_TABSIZE ))->SetRange( 1, 10 );

	m_comboCsetL.AddString( CDocText::s_pszEncodingDefault );
	m_comboCsetR.AddString( CDocText::s_pszEncodingDefault );
	m_comboCsetL.AddString( CDocText::s_pszEncodingUnicode );
	m_comboCsetR.AddString( CDocText::s_pszEncodingUnicode );
	m_comboCsetL.AddString( CDocText::s_pszEncodingUnicodeRev );
	m_comboCsetR.AddString( CDocText::s_pszEncodingUnicodeRev );
	m_comboCsetL.AddString( CDocText::s_pszEncodingUTF8 );
	m_comboCsetR.AddString( CDocText::s_pszEncodingUTF8 );
	m_comboCsetL.AddString( CDocText::s_pszEncodingXUTF8 );
	m_comboCsetR.AddString( CDocText::s_pszEncodingXUTF8 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
