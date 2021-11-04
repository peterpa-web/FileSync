#pragma once
#include "TaskIdle.h"

class CTaskIdleUpd : public CTaskIdle
{
public:
	CTaskIdleUpd();
	~CTaskIdleUpd();
	static CTaskIdleUpd* New(CDualTreeDir *pTree, TREEPOS posParent);
	virtual void Delete();
	virtual int ProcessStep();
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }

protected:
	BOOL m_bArcExpand;

private:
    static CStorage<CTaskIdleUpd> s_storeTasks;
};

