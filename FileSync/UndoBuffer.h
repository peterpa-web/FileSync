#pragma once
#include "afxtempl.h"

class CUndoTask;

class CUndoBuffer
{
public:
	CUndoBuffer(void);
	~CUndoBuffer(void);
	BOOL CanUndo(void);
	BOOL CanRedo(void);

protected:
	CTypedPtrArray< CPtrArray, CUndoTask* > m_apTask;
	int m_nTask;
	INT_PTR m_nMaxSize;
	INT_PTR m_nCurrSize;

public:
	BOOL AddTask(CUndoTask* pTask);
	BOOL Undo(void);
	BOOL Redo(void);
	void RemoveAll(void);
	void RemoveAt(int nStart, int nCount = -1);
	void CleanModifiedFlag(int nChan);
	void SetMaxSize( INT_PTR nSize ) { m_nMaxSize = nSize; }
};
