#pragma once


// CDialogReplace-Dialogfeld

class CDialogReplace : public CDialog
{
	DECLARE_DYNAMIC(CDialogReplace)

public:
	CDialogReplace(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CDialogReplace();

// Dialogfelddaten
	enum { IDD = IDD_REPLACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
};
