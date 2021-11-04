#pragma once
#include "DualTreeDir.h"

#include "TaskBase.h"

class CViewDirItem;
class CDocDirArchive;

class CTaskDirDep :
	public CTaskBase
{
public:
	CTaskDirDep();
	~CTaskDirDep();
	static CTaskDirDep* New(BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, TREEPOS posParent, 
		int nRecurr, int nSortType, BOOL *pbForce);
	virtual void Delete();
	virtual UINT Process(CThreadBack *pThreadBack);

	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }
	static void CreateSubDoc(CViewDirItem &d, int nSide);
	static CDocDirArchive* NewDocDirArcRoot( int nIcon, const CString &strName, CDocDir *pParent, DOCPOS posParent );

protected:
	CDualTreeDir *m_pTree;
	CDualTreeDirData *m_pTreeData;
	TREEPOS m_posParent;
	int m_nRecurr;
	int m_nSortType;
	BOOL m_bForce[2];

	void ProcessStep(CThreadBack *pThreadBack, TREEPOS pos, CViewDirItem &d);

private:
    static CStorage<CTaskDirDep> s_storeTasks;
};

