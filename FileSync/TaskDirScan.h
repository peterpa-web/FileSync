#pragma once
#include "DualTreeDir.h"

#include "TaskBase.h"

//#define WM_UPD_TREE  (WM_USER + 1)

class CTaskIdleDel;

class CTaskDirScan :
	public CTaskBase
{
public:
	CTaskDirScan();
	~CTaskDirScan();
	static CTaskDirScan* New(BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, TREEPOS posParent, int nSide, 
		CDocDir *pDocL, CDocDir *pDocR, int nRecurr, int nSortType, BOOL bForce = FALSE);
	virtual void Delete();
	virtual UINT Process(CThreadBack *pThreadBack);
	static void AssertEmpty() { ASSERT( s_storeTasks.GetCount() == 0 ); }
	static POSITION CreateTasks( CThreadBack *pThreadBack, BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent, 
			CDocDir *pDocL, CDocDir *pDocR, int nRecurr, int nSortType, POSITION posAfter, BOOL *pbForce = NULL );
	BOOL IsCanceled();
	void SetProgress( __int64 nDone );


protected:
	CThreadBack * m_pThreadBack;
	CDualTreeDir* m_pTree;
	CDualTreeDirData* m_pTreeData;
	TREEPOS m_posParent;
	CDocDir *m_pDocL;
	CDocDir *m_pDocR;
	int m_nRecurr;
	int m_nSortType;
	BOOL m_bForce;
	CTaskIdleDel *m_pTaskIdleDel;

	int UpdateDirect( CDocDir *pDoc, __int64 &nMaxFileSize );
	void ClearItemRecursive( TREEPOS pos );
	void UpdateParentMark( TREEPOS posParent );
	int UpdateItems( TREEPOS posChild, CDocDir *pDoc );
	void UpdateSorting( TREEPOS posChild );
	int InsertNewItems( TREEPOS posChild, CDocDir *pDocL, CDocDir *pDocR );

private:
    static CStorage<CTaskDirScan> s_storeTasks;
};

