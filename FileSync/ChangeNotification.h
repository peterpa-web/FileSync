#pragma once
#include "afxmt.h"

#define MAX_CHANGE_HANDLES 4

class CChangeNotification
{
public:
	CChangeNotification(void);
	~CChangeNotification(void);
	BOOL MonitorDir(
		LPCTSTR lpPathName,    // directory name
		BOOL bWatchSubtree,    // monitoring option
		DWORD dwNotifyFilter   // filter conditions
	);
	BOOL MonitorFile(
		LPCTSTR lpPathName,    // directory name
		LPCTSTR lpFileName     // file name
	);
	BOOL MonitorEnd();
	static void Cleanup();			// to be called from ExitInstance()
	void Enable( BOOL bEnable = TRUE );
	BOOL MonitorStart();
	BOOL MonitorRestart();
	LONG WhatIsChanged();		// Query and reset notification 1=size, 2=RO
	void Signal();

private:
	static void AddHandle( CChangeNotification* pObj );
	static void RemoveHandle( CChangeNotification* pObj );
	static void RemoveHandleRst( CChangeNotification* pObj );
	static void Changed( CChangeNotification* pObj );

protected:
//	enum Intr { None, Add, Remove, Exit };
	enum Intr { None, Exit };

	HANDLE m_hChangeHandle;
	CString m_strPath;
	CString m_strFile;
	BOOL m_bWatchSubtree;
	DWORD m_dwNotifyFilter;
	struct _stati64 m_fs;
	BOOL m_bEnabled;
	LONG m_lChanged;
	BOOL m_bRestart;	// restart pendig

	static BOOL Start( CChangeNotification* pObj );
	static BOOL Stop( CChangeNotification* pObj );
	static BOOL CheckWait(DWORD dwWait);
	static UINT ThreadProc( LPVOID pParam );

	static HANDLE s_hChangeHandles[MAX_CHANGE_HANDLES+1];
	static CChangeNotification* s_pObj[MAX_CHANGE_HANDLES+1];
	static int s_nHandles;
	static CChangeNotification* s_pObjRst[MAX_CHANGE_HANDLES];
	static int s_nHandlesRst;
	static CWinThread* s_pThread;
	static Intr s_intr;
	static CCriticalSection s_CS;
};
