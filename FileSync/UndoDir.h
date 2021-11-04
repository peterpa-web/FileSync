#pragma once
#include "UndoTask.h"

class CViewDir;

class CUndoDir :
	public CUndoTask
{
	friend class CViewDir;

public:
	CUndoDir(CViewDir *pView );
	virtual ~CUndoDir(void);
	virtual BOOL DoFirst(void);
	virtual BOOL Do(void);
	virtual BOOL Undo(void);
	virtual INT_PTR GetSize();
	virtual void SetModifiedFlag(int nChan, BOOL bFlag);

protected:
	CViewDir *m_pView;
	CString m_strPath[2];
	CString m_strOldPath[2];
//	CRuntimeClass* m_pDocClass[2];		PP 050331
//	CRuntimeClass* m_pOldDocClass[2];
	CDocTemplFileSync* m_pDocTemplate[2];
	CDocTemplFileSync* m_pOldDocTemplate[2];

//	BOOL DoSide( int nSide, const CString &strPath, CRuntimeClass *pDocClass );
	BOOL DoSide( int nSide, const CString &strPath, CDocTemplFileSync *pDocTemplate );
};
