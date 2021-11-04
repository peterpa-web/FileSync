#pragma once
#include "afxwin.h"


// CAssocDlg dialog

class CDocTemplFileSync;

class CAssocDlg : public CDialog
{
	DECLARE_DYNAMIC(CAssocDlg)

public:
	CAssocDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAssocDlg();

// Dialog Data
	enum { IDD = IDD_ASSOC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_strInitPath;
	CEdit m_editExt;
	CListBox m_listExt;
//	CListBox m_listView;
	CComboBox m_comboView;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDel();
	afx_msg void OnEnChangeEditExt();
	afx_msg void OnLbnSelchangeListExt();
protected:
	virtual void OnOK();
public:
//	afx_msg void OnLbnSelchangeListView();
//	afx_msg void OnLbnSetfocusListView();
	afx_msg void OnCbnSelchangeComboView();
};
