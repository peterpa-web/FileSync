#pragma once
#include "TaskIdle.h"

class CTaskIdleExp : public CTaskIdle
{
public:
	CTaskIdleExp();
	~CTaskIdleExp();
	static CTaskIdleExp* New(CDualTreeDir *pTree, TREEPOS posParent);
	virtual void Delete();
	virtual int ProcessStep();
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }


private:
    static CStorage<CTaskIdleExp> s_storeTasks;
};

