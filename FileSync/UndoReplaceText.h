#pragma once
#include "UndoViewText.h"

class CUndoReplaceText :
	public CUndoViewText
{
public:
	CUndoReplaceText(CViewText *pView, HGLOBAL hg);
	virtual ~CUndoReplaceText(void);
	virtual BOOL DoFirst(void);
	virtual BOOL Do(void);
	virtual BOOL Undo();
	virtual INT_PTR GetSize();

protected:
	HGLOBAL m_hg;
	CStringArray m_astrInsLines;

};
