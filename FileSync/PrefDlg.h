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
	int m_nTabSize = 0;
	BOOL m_bIgnSpaces = FALSE;
	BOOL m_bBomLeft = FALSE;
	BOOL m_bBomRight = FALSE;
	BOOL m_bUnixLeft = FALSE;
	BOOL m_bUnixRight = FALSE;
	BOOL m_bReadOnlyLeft = FALSE;
	BOOL m_bReadOnlyRight = FALSE;
	virtual BOOL OnInitDialog();
	// Characterset left
	CComboBox m_comboCsetL;
	// Characterset Right
	CComboBox m_comboCsetR;
	CString m_strEncodingL;
	CString m_strEncodingR;
	BOOL m_bFileROLeft = FALSE;
	BOOL m_bFileRORight = FALSE;
};
