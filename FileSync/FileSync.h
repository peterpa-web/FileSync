// FileSync.h : main header file for the FileSync application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CFileSyncApp:
// See FileSync.cpp for the implementation of this class
//

class CFileSyncApp : public CWinApp
{
public:
	CFileSyncApp();
	CString m_strHelpPath;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);

protected:
	void SplitCmdLine( const CString& csCmd, CStringArray &csArray );
};

extern CFileSyncApp theApp;