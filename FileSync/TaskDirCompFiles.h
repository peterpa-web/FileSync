#pragma once

#include "TaskBase.h"

class CDualTreeDir;
class CDualTreeDirData;

class CTaskDirCompFiles :
	public CTaskBase
{
public:
	CTaskDirCompFiles();
	~CTaskDirCompFiles();
	static CTaskDirCompFiles* New(BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, TREEPOS posParent);
	virtual void Delete();
	virtual UINT Process(CThreadBack *pThreadBack);
	virtual int Compare( CTaskBase *pTask );
	virtual __int64 NewProgrBytesAll();
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }
	BOOL IsCanceled();
	void SetProgress( __int64 nDone );

protected:
	CDualTreeDir *m_pTree;
	CDualTreeDirData *m_pTreeData;
	TREEPOS m_posParent;
	int m_nRecurr;
	int m_nSortType;
	CThreadBack * m_pThreadBack;
	__int64 m_nTotal;

	void UpdateParentMark( TREEPOS pos, CThreadBack *pThreadBack );

private:
    static CStorage<CTaskDirCompFiles> s_storeTasks;
};

