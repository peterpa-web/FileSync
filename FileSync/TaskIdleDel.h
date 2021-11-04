#pragma once
#include "TaskIdle.h"

class CTaskIdleDel: public CTaskIdle
{
public:
	CTaskIdleDel();
	~CTaskIdleDel();
	static CTaskIdleDel* New(CDualTreeDir *pTree, TREEPOS posParent);
	void AddPos( TREEPOS pos );
	virtual int ProcessStep();
	virtual void Delete();
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }

protected:
	CList<TREEPOS> *m_pListPos;

private:
    static CStorage<CTaskIdleDel> s_storeTasks;
};

