#pragma once
#include "UndoViewText.h"

class CUndoInsertAfter :
	public CUndoViewText
{
public:
	CUndoInsertAfter(CViewText *pView);
	virtual ~CUndoInsertAfter(void);
	virtual BOOL Do();
	virtual BOOL Undo();
};
