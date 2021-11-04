#pragma once
#include "afxwin.h"


// CPrefDlg dialog

class CPrefDlg : public CDialog
{
	DECLARE_DYNAMIC(CPrefDlg)

public:
	CPrefDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPrefDlg();

// Dialog Data
	enum { IDD = IDD_PREF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nTabSize;
	BOOL m_bIgnSpaces;
	BOOL m_bUnixLeft;
	BOOL m_bUnixRight;
	BOOL m_bReadOnlyLeft;
	BOOL m_bReadOnlyRight;
	virtual BOOL OnInitDialog();
	// Characterset left
	CComboBox m_comboCsetL;
	// Characterset Right
	CComboBox m_comboCsetR;
	CString m_strEncodingL;
	CString m_strEncodingR;
	BOOL m_bFileROLeft;
	BOOL m_bFileRORight;
};
