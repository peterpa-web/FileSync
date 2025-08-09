#pragma once
//#include "TitleTip.h"
#include "TitleTipListBox.h"

// CComboBoxDir

class CDocTemplFileSync;

class CComboBoxDir : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxDir)

public:
	CComboBoxDir();
	virtual ~CComboBoxDir();
	virtual BOOL Create(
		DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd,
		UINT nID 
	);
	void SetSide( int nSide ) { m_nSide = nSide; }
	void SetTemplate( CDocTemplFileSync *pTemplate ) { m_pTemplate = pTemplate; }
	void SetText( LPCTSTR lpsz );
	void StoreList();
	CString RestoreList();
	HWND GetEditWnd();
	void Highlight(BOOL b);
	void Warn( BOOL b ) { m_bWarn = b; }
	void SetDirMode() { m_bDirMode = TRUE; }
//	void ClearListTop();

protected:
	DECLARE_MESSAGE_MAP()
    BOOL m_bAutoComplete;
	int m_nSide;
	BOOL m_bDirMode;
	CDocTemplFileSync *m_pTemplate;
	CTitleTipListBox	m_listbox; 
	BOOL m_bWarn;
	CBrush m_brushBk;

    CTitleTip m_TitleTip; // TitleTip that gets displayed when necessary
	CPoint m_LastMouseMovePoint; // Last position of mouse cursor
    BOOL m_bMouseCaptured; // Is mouse captured?

    // This method should be overridden by an owner-draw listbox.
    virtual int GetIdealItemRect(int nIndex, LPRECT lpRect);

    void AdjustTitleTip(int nNewIndex);
    void CaptureMouse();
    BOOL IsAppActive();
	void DoOpen( CString strFile, BOOL bDirMode );

public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnCbnSelendok();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnEditupdate();
	BOOL IsDir( const CString &strPath ) const;
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};


