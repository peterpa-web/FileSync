#pragma once
#include "UndoViewText.h"

class CUndoInsertBefore :
	public CUndoViewText
{
public:
	CUndoInsertBefore(CViewText *pView);
	virtual ~CUndoInsertBefore(void);
	virtual BOOL Do();
	virtual BOOL Undo();
};
