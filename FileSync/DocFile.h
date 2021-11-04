#pragma once
#include "DocFileSync.h"

class CDocFile :
	public CDocFileSync
{
	DECLARE_DYNAMIC(CDocFile)

public:
	CDocFile(void);
	virtual ~CDocFile(void);
//	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData);
	virtual BOOL Refresh( int nSide, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );

protected:
	LPPROGRESS_ROUTINE m_lpProgressRoutine;
	LPVOID m_pProgressMan;

	void SetProgressRoutine( LPPROGRESS_ROUTINE lpRoutine, LPVOID lpData ) { m_lpProgressRoutine = lpRoutine; m_pProgressMan = lpData; }
	void ResetProgressRoutine() { m_lpProgressRoutine = NULL; m_pProgressMan = NULL; }
};

