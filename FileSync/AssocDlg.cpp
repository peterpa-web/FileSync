// AssocDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxtempl.h"
#include "FileSync.h"
#include "DocTemplFileSync.h"
#include "AssocDlg.h"


// CAssocDlg dialog

IMPLEMENT_DYNAMIC(CAssocDlg, CDialog)
CAssocDlg::CAssocDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAssocDlg::IDD, pParent)
{
}

CAssocDlg::~CAssocDlg()
{
}

void CAssocDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EXT, m_editExt);
	DDX_Control(pDX, IDC_LIST_EXT, m_listExt);
//	DDX_Control(pDX, IDC_LIST_VIEW, m_listView);
	DDX_Control(pDX, IDC_COMBO_VIEW, m_comboView);
}


BEGIN_MESSAGE_MAP(CAssocDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnBnClickedButtonDel)
	ON_EN_CHANGE(IDC_EDIT_EXT, OnEnChangeEditExt)
	ON_LBN_SELCHANGE(IDC_LIST_EXT, OnLbnSelchangeListExt)
//	ON_LBN_SELCHANGE(IDC_LIST_VIEW, OnLbnSelchangeListView)
//	ON_LBN_SETFOCUS(IDC_LIST_VIEW, OnLbnSetfocusListView)
ON_CBN_SELCHANGE(IDC_COMBO_VIEW, OnCbnSelchangeComboView)
END_MESSAGE_MAP()


// CAssocDlg message handlers

BOOL CAssocDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_listExt.SetTabStops( 50 );

	POSITION posDocTemp = AfxGetApp()->GetFirstDocTemplatePosition();;
	while (posDocTemp != NULL)
	{
		CDocTemplate* pTemplate = AfxGetApp()->GetNextDocTemplate(posDocTemp);
		ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
		CDocTemplFileSync *pDocTempl = (CDocTemplFileSync *)pTemplate;
		UINT nRes = pDocTempl->GetResourceID();
		if ( nRes != IDR_VIEWDIR && nRes != IDR_VIEWHEX )
		{
			CString strView( (LPCTSTR)nRes );
			strView = strView.Mid( 9 );
			m_comboView.AddString( strView );
			CStringArray &astr = pDocTempl->GetExtensionsArray();
			for ( int n = 0; n < astr.GetSize(); ++n )
				m_listExt.AddString( astr[n].Mid(1) + '\t' + strView );
		}
	}
	int p = m_strInitPath.Find('.');
	if ( p >= 0 ) {
		CString strExt = m_strInitPath.Mid(p+1).MakeLower();
		m_editExt.SetWindowText(strExt);
		// locate in m_listExt from OnEnChangeEditExt();
	}
	else
		m_comboView.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAssocDlg::OnBnClickedButtonAdd()
{
	m_listExt.SelItemRange( FALSE, 0, m_listExt.GetCount()-1 );
	CString strExt;
	m_editExt.GetWindowText( strExt );
	CString strView;
	m_comboView.GetWindowText( strView );
	int nItem = m_listExt.AddString( strExt + '\t' + strView );
	m_listExt.SetSel( nItem );
	GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( FALSE );
	GetDlgItem( IDOK )->EnableWindow( TRUE );
}

void CAssocDlg::OnBnClickedButtonDel()
{
	// Get the indexes of all the selected items.
	int nCount = m_listExt.GetSelCount();
	CArray<int,int> anSel;

	anSel.SetSize( nCount );
	m_listExt.GetSelItems( nCount, anSel.GetData() );

	CStringArray astr;
	int n;
	for ( n = 0; n < anSel.GetSize(); ++n )
	{
		CString str;
		m_listExt.GetText( anSel[n], str ); 
		astr.Add( str );
	}
	for ( n = 0; n < astr.GetSize(); ++n )
		m_listExt.DeleteString( m_listExt.FindStringExact( -1, astr[n] ) );

	GetDlgItem( IDC_BUTTON_DEL )->EnableWindow( FALSE );
	GetDlgItem( IDOK )->EnableWindow( TRUE );
}

void CAssocDlg::OnEnChangeEditExt()
{
	static BOOL bBusy = FALSE;
	if ( bBusy )
		return;

	CString strExt;
	m_comboView.GetWindowText( strExt );
	if ( strExt.IsEmpty() )
	{
		GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( FALSE );
//		return;
	}
	m_editExt.GetWindowText( strExt );
	CString strExtTrim = strExt;
	strExtTrim.TrimLeft();	// VC6
	strExtTrim.TrimRight();
	//strExtTrim.Trim();
	strExtTrim.Remove( '.' );
	strExtTrim.Remove( '\\' );
	strExtTrim.Remove( '/' );
	strExtTrim.Remove( '\"' );
	strExtTrim.Remove( '\'' );
	strExtTrim.Remove( '*' );
	strExtTrim.Remove( '?' );
	if ( strExt != strExtTrim )
	{
		bBusy = TRUE;
		DWORD dwSel = m_editExt.GetSel();
		m_editExt.SetWindowText( strExtTrim );
		m_editExt.SetSel( dwSel );
		bBusy = FALSE;
	}
	if ( strExtTrim.IsEmpty() )
	{
		GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( FALSE );
		return;
	}
	int nItem = m_listExt.FindString( -1, strExtTrim + '\t' );
	m_listExt.SelItemRange( FALSE, 0, m_listExt.GetCount()-1 );		// unselect all
	if (nItem != LB_ERR) {
		m_listExt.SetSel(nItem);
		m_listExt.SetTopIndex(nItem);
		CString str;
		m_listExt.GetText(nItem, str);
		int nTab = str.Find('\t');
		str = str.Mid(nTab+1);
		m_comboView.SelectString(-1, str);
	}
	GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( nItem == LB_ERR );
	GetDlgItem( IDC_BUTTON_DEL )->EnableWindow( nItem != LB_ERR );
}

void CAssocDlg::OnLbnSelchangeListExt()
{
	int nSel = m_listExt.GetSelCount();
	if ( nSel == 1 ) {
		CArray<int,int> anSel;
		anSel.SetSize( nSel );
		m_listExt.GetSelItems( nSel, anSel.GetData() );
		CString str;
		m_listExt.GetText(anSel[0], str);
		int nTab = str.Find('\t');
		m_editExt.SetWindowText(str.Left(nTab));
		return;
	}
	else
		m_editExt.SetWindowText(_T(""));
	GetDlgItem( IDC_BUTTON_DEL )->EnableWindow( nSel > 0 && nSel < m_listExt.GetCount() );
}

void CAssocDlg::OnOK()
{
	POSITION posDocTemp = AfxGetApp()->GetFirstDocTemplatePosition();;
	while (posDocTemp != NULL)
	{
		CDocTemplate* pTemplate = AfxGetApp()->GetNextDocTemplate(posDocTemp);
		ASSERT_KINDOF(CDocTemplFileSync, pTemplate);
		CDocTemplFileSync *pDocTempl = (CDocTemplFileSync *)pTemplate;
		UINT nRes = pDocTempl->GetResourceID();
		if ( nRes != IDR_VIEWDIR && nRes != IDR_VIEWHEX )
		{
			CString strView( (LPCTSTR)nRes );
			strView = '\t' + strView.Mid( 9 );
			int nLenView = strView.GetLength();
			CStringArray &astr = pDocTempl->GetExtensionsArray();
			astr.RemoveAll();

			for ( int n = 0; n < m_listExt.GetCount(); ++n )
			{
				CString str;
				m_listExt.GetText( n, str );
				if ( str.Right(nLenView) == strView )
				{
					astr.Add( CString( '.' ) + str.Left( str.GetLength() - nLenView ) );
				}
			}
			pDocTempl->WriteProfile();
		}
	}
	CDialog::OnOK();
}

void CAssocDlg::OnCbnSelchangeComboView()
{
	OnEnChangeEditExt();
}
