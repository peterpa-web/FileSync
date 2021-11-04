#pragma once
#include "resource.h"


// CDlgOverwriteNewer dialog

class CDlgOverwriteNewer : public CDialog
{
	DECLARE_DYNAMIC(CDlgOverwriteNewer)

public:
	CDlgOverwriteNewer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgOverwriteNewer();
	CString m_strFileName;

// Dialog Data
	enum { IDD = IDD_OVERWRITE_NEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRetry();
	virtual BOOL OnInitDialog();
};
