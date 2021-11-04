#include "StdAfx.h"
#include "TaskIdleInv.h"

CStorage<CTaskIdleInv> CTaskIdleInv::s_storeTasks( 100, _T("CTaskIdleInv") );

CTaskIdleInv::CTaskIdleInv() 
{
	m_pStore = &s_storeTasks;
}

CTaskIdleInv::~CTaskIdleInv(void)
{
}

CTaskIdleInv* CTaskIdleInv::New( CDualTreeDir *pTree, TREEPOS posParent, BOOL bShow ) 
{
	POSITION posTask = s_storeTasks.New();
    CTaskIdleInv *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( pTree, posParent );
	pTask->m_bShow = bShow;
#ifdef _DEBUG
	if ( pTree->HasDirIcon(posParent) || s_storeTasks.GetCount() == 0 ) {
		TRACE3("CTaskIdleInv::New %s %d c=%d\n", pTree->GetItemNameDebug(posParent), bShow, s_storeTasks.GetCount() );
	}
#endif
    return pTask;
}

int CTaskIdleInv::Compare( CTaskIdle *pTask )
{
	CTaskIdleInv* pTaskOther = (CTaskIdleInv*) pTask;
	if ( pTaskOther->m_pStore != m_pStore )
		return 1;
	if ( pTaskOther->m_posParent != m_posParent )
		return 1;
	if ( pTaskOther->m_bShow != m_bShow )
		return 1;
	TRACE2("CTaskIdleInv::Compare match %s %d\n", m_pTree->GetItemNameDebug(m_posParent), m_bShow );
	return 0;
}

void CTaskIdleInv::Delete()
{
	TRACE3( "CTaskIdleInv::Delete %d %s c=%d\n", m_bShow, m_pTree->GetItemNameDebug( m_posParent ), s_storeTasks.GetCount() );
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

int CTaskIdleInv::ProcessStep(void)
{
	if ( m_bShow ) {
		m_pTree->UnlockWindowUpdate();
		m_pTree->EnsureVisible(m_posParent);
	} else
		m_pTree->InvalidateItem(m_posParent);

	TRACE2("CTaskIdleInv::ProcessStep %s %d\n", m_pTree->GetItemNameDebug(m_posParent), m_bShow );
	return 0;	// done
}
