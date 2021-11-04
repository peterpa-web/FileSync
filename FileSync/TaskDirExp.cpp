#include "StdAfx.h"
#include "DualTreeDirData.h"
#include "ThreadBack.h"
#include "TaskIdleExp.h"

#include "TaskDirExp.h"

CStorage<CTaskDirExp> CTaskDirExp::s_storeTasks( 100, _T("CTaskDirExp") );


CTaskDirExp::CTaskDirExp()  
{
}

CTaskDirExp::~CTaskDirExp(void)
{
}

CTaskDirExp* CTaskDirExp::New(BOOL bMain, 
			CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent, int nRecurr)
{
	POSITION posTask = s_storeTasks.New();
    CTaskDirExp *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( bMain );
	TRACE1( "CTaskDirExp::New %s\n", pTreeData->GetItemNameDebug( posParent ) );
	pTask->m_posParent = posParent;
	pTask->m_nSide = 2;
	pTask->m_pTree = pTree;
	pTask->m_pTreeData = pTreeData;
	pTask->m_nRecurr = nRecurr;
    return pTask;
}

void CTaskDirExp::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

UINT CTaskDirExp::Process(CThreadBack *pThreadBack)
{
	TRACE2( "CTaskDirExp::Process %d %s\n", m_nRecurr, m_pTreeData->GetItemNameDebug( m_posParent ) );
	BOOL bExpand = FALSE;
    CViewDirItem &d = m_pTreeData->GetItemData( m_posParent );
	if ( m_nRecurr >= 1000 )	// expand all
		bExpand = TRUE;
	else if ( m_nRecurr < 0 && d.IsPresent(0) && d.IsPresent(1) )	// expand part
		bExpand = TRUE;
	if ( !bExpand ) {
		return 0;	// finished
	}
	CTaskIdle *pTaskIdleExp = CTaskIdleExp::New( m_pTree, m_posParent );
	pThreadBack->AddIdleTask( pTaskIdleExp );

	POSITION posAfter = NULL;
    TREEPOS pos = m_pTreeData->GetFirstChildPos( m_posParent );
    while ( pos != NULL ) {
		bExpand = FALSE;
        CViewDirItem &d = m_pTreeData->GetItemData( pos );
		if ( d.HasDirOrArcIcon() ) {
			int nRecurr;
			if ( m_nRecurr >= 1000 ) {	// expand all
				bExpand = TRUE;
				nRecurr = m_nRecurr - 1;
			} else if ( m_nRecurr < 0 && d.IsPresent(0) && d.IsPresent(1) )	{ // expand part
				bExpand = TRUE;
				nRecurr = m_nRecurr + 1;
			}
			if ( bExpand ) {
				CTaskDirExp *pTaskDirExp = CTaskDirExp::New( m_bMain, m_pTree, m_pTreeData, pos, nRecurr );
				if ( posAfter == NULL )
					posAfter = pThreadBack->AddTask( pTaskDirExp, this );
				else
					posAfter = pThreadBack->AddTask( pTaskDirExp, posAfter );
			}
		}
		pos = m_pTreeData->GetNextSiblingPos( pos );
    }

	return 0;
}

