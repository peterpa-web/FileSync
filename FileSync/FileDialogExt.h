#pragma once


// CFileDialogExt

class CFileDialogExt : public CFileDialog
{
	//DECLARE_DYNAMIC(CFileDialogExt)

public:
	CFileDialogExt(BOOL bOpenFileDialog, // TRUE für FileOpen, FALSE für FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CFileDialogExt();
	afx_msg void OnSelChangeType();
	CString GetType() { return m_strType; }
	void SetType( const CString &strType ) { m_strType = strType; }

protected:
	DECLARE_MESSAGE_MAP()
	CString m_strType;

public:
	virtual BOOL OnInitDialog();
protected:
	virtual BOOL OnFileNameOK();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};


