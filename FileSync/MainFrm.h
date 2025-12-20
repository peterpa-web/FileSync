// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include <afxframewndex.h>
#include "ToolBarFixed.h"
#include "ToolBarSearch.h"
#include "REBProgressDialog.h"

//class CDirCompFrmWnd;
class CViewFileSync;

class CMainFrame : public CFrameWndEx
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	BOOL OnIdle(LONG lCount);
	void SetActiveView( CView *pViewNew, BOOL bNotify = TRUE );
	CString GetSearchText() { return m_wndToolBarSearch.GetSearchText(); }
	BOOL UpdateMessageText( const CString &strMsg, int nProgress = 0 );
	BOOL IsAppActive() { return m_bActive; }
	CToolBarSearch& GetToolBarSearch() { return m_wndToolBarSearch; }
	REBProgressManager* GetProgressMan() { return &m_progressMan; }
	CViewFileSync* GetNewClient() const { return m_pNewClient; }

protected:
	void SaveWinPos();
	void SwitchToolBar(int nToolbarID);
	REBProgressManager m_progressMan;
	CViewFileSync* m_pNewClient;

public:
	BOOL RestoreWinPos();

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCStatusBar m_wndStatusBar;
	CMap<int, int, CToolBarFixed*, CToolBarFixed*&> m_mapToolBarCache;
	CToolBarSearch m_wndToolBarSearch;
	int m_nToolbarID;
	BOOL m_bIndicatorLeft;
	BOOL m_bIndicatorRight;
	CString m_strMsg;
	BOOL m_bActive;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnToolbarReset(WPARAM wparm, LPARAM);
};


