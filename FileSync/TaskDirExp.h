#pragma once
#include "DualTreeDir.h"

#include "TaskBase.h"

class CTaskDirExp :
	public CTaskBase
{
public:
	CTaskDirExp();
	~CTaskDirExp();
	static CTaskDirExp* New(BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, TREEPOS posParent, int nRecurr);
	virtual void Delete();
	virtual UINT Process(CThreadBack *pThreadBack);
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }


protected:
	CDualTreeDir *m_pTree;
	CDualTreeDirData *m_pTreeData;
	TREEPOS m_posParent;
	int m_nRecurr;

private:
    static CStorage<CTaskDirExp> s_storeTasks;
};

