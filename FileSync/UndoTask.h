#pragma once

class CUndoTask
{
public:
	CUndoTask(void);
	virtual ~CUndoTask(void);
	virtual BOOL DoFirst(void) = 0;
	virtual BOOL Do(void) = 0;
	virtual BOOL Undo(void) = 0;
	virtual INT_PTR GetSize() = 0;
	virtual void SetModifiedFlag(int nChan, BOOL bFlag) = 0;
};
