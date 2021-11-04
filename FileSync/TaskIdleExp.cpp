#include "StdAfx.h"
#include "TaskIdleExp.h"


CStorage<CTaskIdleExp> CTaskIdleExp::s_storeTasks( 100, _T("CTaskIdleExp") );

CTaskIdleExp::CTaskIdleExp() 
{
}

CTaskIdleExp::~CTaskIdleExp(void)
{
}

CTaskIdleExp* CTaskIdleExp::New( CDualTreeDir *pTree, TREEPOS posParent ) 
{
	POSITION posTask = s_storeTasks.New();
    CTaskIdleExp *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( pTree, posParent );
	TRACE1("CTaskIdleExp::New %s %d\n", pTree->GetItemNameDebug(posParent) );
    return pTask;
}

void CTaskIdleExp::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

int CTaskIdleExp::ProcessStep(void)
{
	TRACE1("CTaskIdleExp::ProcessStep %s\n", m_pTree->GetItemNameDebug(m_posParent));
//			m_tree.ShowWindow( SW_HIDE );
	m_pTree->LockWindowUpdate();
	BOOL rc = m_pTree->Expand( m_posParent );
//		if ( m_bExpand )
//			m_tree.ShowWindow( SW_NORMAL );
	return 0;	// done
}
