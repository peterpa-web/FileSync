#pragma once
#include "UndoViewText.h"

class CUndoReplace :
	public CUndoViewText
{
public:
	CUndoReplace(CViewText *pView);
	virtual ~CUndoReplace(void);
	virtual BOOL DoFirst();
	virtual BOOL Do();
	virtual BOOL Undo();
	virtual INT_PTR GetSize();

protected:
	CStringArray m_astrDelLines;
};
