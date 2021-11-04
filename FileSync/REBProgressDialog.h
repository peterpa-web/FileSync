#pragma once

#include <afxcmn.h> // for CProgressCtrl
#include "resource.h"

#define REBPROGRESSMAXSTRING 100
#define REBPROGRESSMAXSTATIC 3 // orig 3

typedef struct tagREBPROGRESSDATA
{
	TCHAR cCaption[REBPROGRESSMAXSTRING + 1];
	TCHAR cAbortText[REBPROGRESSMAXSTRING + 1];
	TCHAR cStaticText[REBPROGRESSMAXSTATIC][REBPROGRESSMAXSTRING + 1];
	BOOL  bCancelEnabled;
	BOOL  bProgressEnabled;
	BOOL  bVisible;
	int   nProgress;
} REBPROGRESSDATA;

void REBInitializeProgressData(REBPROGRESSDATA* pData);
void REBCopyProgressData(REBPROGRESSDATA* pDataDest, REBPROGRESSDATA* pDataSource);

class REBProgressDialog; // forward declaration
class REBProgressThread; // forward declaration

/////////////////////////////////////////////////////////////////////////////
// REBProgressManager

class REBProgressManager
{
public:
	REBProgressManager();
	virtual ~REBProgressManager();

	void EndProgressDialog();
	void BeginProgressDialog();
	BOOL IsInited() { return m_bInited; }
	BOOL IsProgressDialogActive() { return (m_pThread != NULL); }
	void Exit();
	void Init(HWND hwndParent);

	BOOL GetVisible();
	void SetVisible(BOOL bNewVal);
	LPCTSTR GetAbortText();
	void SetAbortText(LPCTSTR pszText);
	BOOL IsProgressEnabled();
	BOOL IsCancelEnabled();
	void ResetUserAbortFlag();
	BOOL GetUserAbortFlag();
	LPBOOL GetUserAbortFlagPtr();
	void EnableProgress(BOOL bEnable);
	void EnableCancel(BOOL bEnable);
	int GetProgress();
	void SetProgress(int nVal);
	void SetRange(__int64 nVal);
	void AddProgress(__int64 nVal);
	void SetProgress();
	void SetProgressToBase();
	void AddProgressBased(__int64 nVal);
	LPCTSTR GetCaption();
	void SetCaption(LPCTSTR pszCaption);
	LPCTSTR GetStaticText(int nIndex);
	void SetStaticText(int nIndex, LPCTSTR pszText);

protected:
	HWND  m_hwndParent;
	BOOL m_bInited;
	REBProgressThread* m_pThread;
	BOOL m_bVisible;
	int m_nProgr;	// 0..100
	__int64 m_nRange;
	__int64 m_nProgress;
	__int64 m_nBase;
	DWORD m_dwTickStart;

protected:
	void PreAccess(int nCritSec);
	void PostAccess(int nCritSec);
	void NotifyChange();
	void WaitForProgressDialog();
	void CheckAndPump();
};

/////////////////////////////////////////////////////////////////////////////
// REBProgressThread

class REBProgressThread : public CWinThread
{
public:
	DECLARE_DYNCREATE(REBProgressThread)
	REBProgressThread(HWND hwndParent);
	REBProgressThread(); // constructor required by DECLARE_DYNCREATE macro (not used)
	~REBProgressThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
protected:
	HWND  m_hwndParent;
};

/////////////////////////////////////////////////////////////////////////////
// REBProgressDialog dialog

class REBProgressDialog : public CDialog
{
public:
// Construction
public:
	REBProgressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(REBProgressDialog)
	enum { IDD = IDD_DIALOG_PROGRESS };
	CButton	m_buttonCancel;
	CProgressCtrl	m_ctrlProgress;
	CString	m_csText1;
	CString	m_csText2;
	CString	m_csText3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(REBProgressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void HandleUserAbort();
	void RefreshData(BOOL bInit = FALSE);
	CString m_csAbortText;
//	BOOL m_bUserAbortFlag;
	BOOL m_bProgressEnabled;
	BOOL m_bCancelEnabled;
	BOOL m_bVisible;
	int m_nPos;
	CString m_csCaption;
	REBPROGRESSDATA m_TempData;

	void Reposition();

	// Generated message map functions
	//{{AFX_MSG(REBProgressDialog)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnRefresh(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
