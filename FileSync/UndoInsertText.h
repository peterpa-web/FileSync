#pragma once
#include "UndoViewText.h"

class CUndoInsertText :
	public CUndoViewText
{
public:
	CUndoInsertText(CViewText *pView, HGLOBAL hg);
	virtual ~CUndoInsertText(void);
	virtual BOOL DoFirst(void);
	virtual BOOL Do(void);
	virtual BOOL Undo();
	virtual INT_PTR GetSize();

protected:
	HGLOBAL m_hg;
	CStringArray m_astrInsLines;

};
