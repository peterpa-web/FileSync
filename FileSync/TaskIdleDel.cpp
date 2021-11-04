#include "StdAfx.h"
#include "TaskIdleDel.h"


CStorage<CTaskIdleDel> CTaskIdleDel::s_storeTasks( 100, _T("CTaskIdleDel") );

CTaskIdleDel::CTaskIdleDel() 
{
}

CTaskIdleDel::~CTaskIdleDel()
{
}

CTaskIdleDel* CTaskIdleDel::New( CDualTreeDir *pTree, TREEPOS posParent ) 
{
	POSITION posTask = s_storeTasks.New();
    CTaskIdleDel *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( pTree, NULL );
	TRACE1("CTaskIdleDel::New %s %d\n", pTree->GetItemNameDebug(posParent) );
	pTask->m_pListPos = new CList<TREEPOS> ( 100 );
	pTask->AddPos( posParent );
    return pTask;
}

void CTaskIdleDel::Delete()
{
	delete m_pListPos;
	m_pListPos = NULL;
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

void CTaskIdleDel::AddPos( TREEPOS pos )
{
	m_pListPos->AddTail( pos );
}

int CTaskIdleDel::ProcessStep()
{
	if ( m_pos == NULL ) {	// 1st step
		m_pos = m_pListPos->GetHeadPosition();
		m_posParent = m_pListPos->GetAt( m_pos );
		TRACE1("CTaskIdleDel::ProcessStep start %s\n", m_pTree->GetItemNameDebug(m_posParent));
		m_pTree->LockWindowUpdate();
	}

	m_posParent = m_pListPos->GetNext( m_pos );
	m_pTree->DeleteItem(m_posParent);

	if ( m_pos == NULL ) {	// finished
		m_pTree->UnlockWindowUpdate();
		TRACE0("CTaskIdleDel::ProcessStep done\n");
		return 0;	// done
	}
	return 1;	// next step should follow
}
