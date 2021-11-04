#include "StdAfx.h"
#include "ThreadBack.h"
#include "TaskDirScan.h"
#include "DualTreeDirData.h"
#include "DocDirZipRoot.h"
#include "DocDirIsoRoot.h"

#include "TaskDirDep.h"

CStorage<CTaskDirDep> CTaskDirDep::s_storeTasks( 100, _T("CTaskDirDep") );

CTaskDirDep::CTaskDirDep()
{
}

CTaskDirDep::~CTaskDirDep()
{
}

CTaskDirDep* CTaskDirDep::New(BOOL bMain, 
			CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent, int nRecurr, int nSortType, BOOL *pbForce)
{
	POSITION posTask = s_storeTasks.New();
    CTaskDirDep *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( bMain );
	TRACE1( "CTaskDirDep::New %s\n", pTreeData->GetItemNameDebug( posParent ) );
	pTask->m_posParent = posParent;
	pTask->m_nSide = 2;
	pTask->m_pTree = pTree;
	pTask->m_pTreeData = pTreeData;
	pTask->m_nRecurr = nRecurr;
	ASSERT( nRecurr != 0 );
	pTask->m_nSortType = nSortType;
	pTask->m_bForce[0] = pbForce[0];
	pTask->m_bForce[1] = pbForce[1];
    return pTask;
}

void CTaskDirDep::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

UINT CTaskDirDep::Process(CThreadBack *pThreadBack)
{
	TRACE1( "CTaskDirDep::Process %s\n", m_pTreeData->GetItemNameDebug( m_posParent ) );
	HWND hwndMsgTarget = pThreadBack->GetHWndOwner();  
	TREEPOS pos = m_pTreeData->GetFirstChildPos( m_posParent );
//	while ( pos != NULL && !m_bCanceled ) 
	while ( pos != NULL && !pThreadBack->IsCanceled() ) 
	{
		CViewDirItem &d = m_pTreeData->GetItemData( pos );
			CDocDir *pd = d.GetParentDoc(0);
			if ( pd != NULL )
				ASSERT_KINDOF( CDocDir, pd );
			pd = d.GetParentDoc(1);
			if ( pd != NULL )
				ASSERT_KINDOF( CDocDir, pd );

		if ( !d.IsDeleted() &&
			 ( d.HasDirIcon() ||
			  (d.HasArcIcon() && d.GetLastChildPos() != NULL ) ) )		// expanded ZIP
		{
			ProcessStep( pThreadBack, pos, d );
		}
		pos = m_pTreeData->GetNextSiblingPos( pos );
	}
	return 0;
}

void CTaskDirDep::ProcessStep(CThreadBack *pThreadBack, TREEPOS pos, CViewDirItem &d )
{
	TRACE2( "CTaskDirDep::ProcessStep %s, %d\n", d.GetName(), m_nRecurr );
	if ( d.GetName() == _T("..") ) {
		d.ResetMarkDirty();
	}
	else {
		CreateSubDoc( d, 0 );
		CreateSubDoc( d, 1 );

		CTaskBase *pTask0 = NULL;
		CTaskBase *pTask1 = NULL;
		int nRecurr = m_nRecurr > 0 ? m_nRecurr - 1 : m_nRecurr + 1;
		if ( m_nRecurr == 1 ) {	// assert preview for all prepared items
			if ( ( d.GetDoc(0) != NULL && d.GetDoc(0)->HasAnyDocs() ) ||
				 ( d.GetDoc(1) != NULL && d.GetDoc(1)->HasAnyDocs() ) ) {
				TRACE0( " nRecurr = 1\n" );
				nRecurr = 1;
			}
		}
		CTaskDirScan::CreateTasks( pThreadBack, m_bMain, m_pTree, m_pTreeData, pos, d.GetDoc(0), d.GetDoc(1), nRecurr, m_nSortType, 
			pThreadBack->GetTaskPos(m_bMain), m_bForce );
	}
}

void CTaskDirDep::CreateSubDoc(CViewDirItem &d, int nSide)
{
	if ( !d.IsValid(nSide) ) {
		ASSERT( d.GetParentDoc(nSide) != NULL );
		d.SetParentPos(nSide, d.GetParentDoc(nSide)->InsertDummy(d.IsDir(), d.GetName()) );
	}
	CDirEntry &de = d.GetDirEntry( nSide );
	if ( de.GetDoc() != NULL )
		return;

	switch ( d.GetIcon() )
	{
	case CDocFileSync::IconDir:
		{
			CDocDir *pDir = d.GetParentDoc(nSide);
			ASSERT_KINDOF( CDocDir, pDir );
			pDir->AssertValid();
			de.SetDoc( pDir->CreateSubDoc( de.GetName(), d.GetParentPos(nSide) ) );
		}
		break;

	default:
		de.SetDoc( NewDocDirArcRoot( d.GetIcon(), de.GetName(), d.GetParentDoc(nSide), d.GetParentPos(nSide) ));
		break;	// no valid icon type
	}
}

CDocDirArchive* CTaskDirDep::NewDocDirArcRoot( int nIcon, const CString &strName, CDocDir *pParent, DOCPOS posParent )
{
	switch ( nIcon )
	{
	case CDocFileSync::IconZipRoot:
		return CDocDirZipRoot::New( strName, pParent, posParent );

	case CDocFileSync::IconIsoRoot:
		return CDocDirIsoRoot::New( strName, pParent, posParent );

	default:
		ASSERT( FALSE );
		return NULL;	// no valid icon type
	}
}
