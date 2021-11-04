#pragma once
#include "afxwin.h"


// CColorsDlg dialog

class CColorsDlg : public CDialog
{
	DECLARE_DYNAMIC(CColorsDlg)

public:
	CColorsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CColorsDlg();
	COLORREF m_acr[5];

	void GetProfile( LPCTSTR pszSection );
	void WriteProfile() const;

// Dialog Data
	enum { IDD = IDD_COLORS };

protected:
	COLORREF m_acrDef[5];
	CString m_strSection;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnBnClickedButtonCx( int nCtrl );

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonC1();
	afx_msg void OnBnClickedButtonC2();
	afx_msg void OnBnClickedButtonC3();
	afx_msg void OnBnClickedButtonC4();
	afx_msg void OnBnClickedButtonC5();
protected:
	virtual void OnOK();
public:
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBnClickedButtonDef();
};
