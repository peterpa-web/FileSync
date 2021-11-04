#pragma once
#include "UndoTask.h"
#include "ViewList.h"
#include "DualListBox.h"

class CViewList;

class CUndoTaskFileSync :
	public CUndoTask
{
public:
	CUndoTaskFileSync(CViewList *pView );
	virtual ~CUndoTaskFileSync(void);
	virtual BOOL DoFirst(void);

protected:
	BOOL PrepareRange();
	void SetSelection( CViewFileSync::Side nSide, int nStart, int nCount );
	CViewFileSync::Side OtherSide() { return ( m_nSide == CViewFileSync::left ? CViewFileSync::right : CViewFileSync::left ); }

	CDualListBox& GetLB() { return m_pView->m_list; }

protected:
	CViewList *m_pView;
	CViewFileSync::Side m_nSide;
	int m_nTopIndex;
	int m_nSelAnchor;
	int m_nSelCount;
};
