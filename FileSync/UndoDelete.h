#pragma once
#include "UndoViewText.h"

class CUndoDelete :
	public CUndoViewText
{
public:
	CUndoDelete(CViewText *pView);
	virtual ~CUndoDelete(void);
	virtual BOOL DoFirst(void);
	virtual BOOL Do();
	virtual BOOL Undo();

	BOOL m_bKeepSel;

protected:
};
