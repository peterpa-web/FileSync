#include "StdAfx.h"
#include "TaskIdleUpd.h"


CStorage<CTaskIdleUpd> CTaskIdleUpd::s_storeTasks( 100, _T("CTaskIdleUpd") );

CTaskIdleUpd::CTaskIdleUpd() 
{
	m_bArcExpand = FALSE;
}


CTaskIdleUpd::~CTaskIdleUpd(void)
{
}

CTaskIdleUpd* CTaskIdleUpd::New( CDualTreeDir *pTree, TREEPOS posParent ) 
{
	POSITION posTask = s_storeTasks.New();
    CTaskIdleUpd *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( pTree, posParent );
	TRACE1("CTaskIdleUpd::New %s\n", pTree->GetItemNameDebug(posParent) );
    return pTask;
}

void CTaskIdleUpd::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

int CTaskIdleUpd::ProcessStep(void)
{
	if ( m_pos == NULL ) {	// 1st step
		m_pos = m_pTree->GetFirstChildPos( m_posParent );
		if ( m_pos == NULL ) {
			if ( m_posParent == NULL ) {
		//		TRACE0("CTaskIdleUpd::ProcessStep empty\n");
				m_pTree->UpdColumnSize();
				return 0;	// done
			}
			return 0;	// no childs
//			TRACE1("CTaskIdleUpd::ProcessStep ERROR %s\n", m_tree.GetItemNameDebug(m_posParent));
//			ASSERT( FALSE );
//			return -1;	// error
		}
		m_bArcExpand = m_pTree->IsArcExpand( m_posParent );
		TRACE1("CTaskIdleUpd::ProcessStep start %s\n", m_pTree->GetItemNameDebug(m_posParent));
		m_pTree->LockWindowUpdate();
	}

	m_pTree->UpdRealItem(m_pos);

	if ( m_pos == NULL ) {	// finished
		m_pTree->UpdColumnSize();
		m_pTree->UnlockWindowUpdate();
//		TRACE0("CTaskIdleUpd::ProcessStep done\n");
		TRACE1("CTaskIdleUpd::ProcessStep done %s\n", m_pTree->GetItemNameDebug(m_posParent));
		if ( m_bArcExpand )
			m_pTree->Expand( m_posParent );
		return 0;	// done
	}
	return 1;	// next step should follow
}
