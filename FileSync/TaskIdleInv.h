#pragma once
#include "TaskIdle.h"

class CTaskIdleInv: public CTaskIdle
{
public:
	CTaskIdleInv();
	~CTaskIdleInv();
	static CTaskIdleInv* New(CDualTreeDir *pTree, TREEPOS posParent, BOOL bShow = FALSE);
	virtual void Delete();
	virtual int ProcessStep(void);
	virtual int Compare( CTaskIdle *pTask );
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }

protected:
	BOOL m_bShow;

private:
    static CStorage<CTaskIdleInv> s_storeTasks;
};

