#include "StdAfx.h"
#include "DualTreeDirData.h"
#include "ThreadBack.h"
#include "TaskDirDep.h"
#include "TaskDirCompFiles.h"
#include "TaskIdleUpd.h"
#include "TaskIdleDel.h"
#include "TaskIdleInv.h"
//#include "DocDirZipRoot.h"
#include "DocManFileSync.h"

#include "TaskDirScan.h"

CStorage<CTaskDirScan> CTaskDirScan::s_storeTasks( 100, _T("CTaskDirScan") );

CTaskDirScan::CTaskDirScan()
{
	m_pThreadBack = NULL;
}

CTaskDirScan::~CTaskDirScan()
{
}

CTaskDirScan* CTaskDirScan::New(BOOL bMain, 
			CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent, int nSide, CDocDir *pDocL, CDocDir *pDocR, 
			int nRecurr, int nSortType, BOOL bForce)
{
	POSITION posTask = s_storeTasks.New();
    CTaskDirScan *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( bMain );
	TRACE3( "CTaskDirScan::New %d %s c=%d\n", nSide, pTreeData->GetItemNameDebug( posParent ), s_storeTasks.GetCount() );
	pTask->m_posParent = posParent;
	pTask->m_nSide = nSide;
	ASSERT( (nSide == 0 ? pDocL : pDocR) != NULL );
	pTask->m_pTree = pTree;
	pTask->m_pTreeData = pTreeData;
	pTask->m_pDocL = pDocL;
	pTask->m_pDocR = pDocR;
	pTask->m_nRecurr = nRecurr;
	pTask->m_nSortType = nSortType;
	pTask->m_bForce = bForce;
	pTask->m_pTaskIdleDel = NULL;
	if ( posParent != NULL && pTreeData->GetItemData(posParent).HasArcIcon() )
		pTask->m_nProgrBytesAll = pTreeData->GetItemData(posParent).GetFileSize(nSide);
    return pTask;
}

void CTaskDirScan::Delete()
{
	TRACE3( "CTaskDirScan::Delete %d %s c=%d\n", m_nSide, m_pTreeData->GetItemNameDebug( m_posParent ), s_storeTasks.GetCount() );
	if ( m_pTaskIdleDel != NULL ) {
		m_pTaskIdleDel->Delete();
		m_pTaskIdleDel = NULL;
	}
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

BOOL CTaskDirScan::IsCanceled() 
{ 
	return m_pThreadBack->IsCanceled( m_nSide ); 
}

DWORD CALLBACK ScanProgressRoutine(
	__in      LARGE_INTEGER TotalFileSize,
	__in      LARGE_INTEGER TotalBytesTransferred,
	__in      LARGE_INTEGER StreamSize,
	__in      LARGE_INTEGER StreamBytesTransferred,
	__in      DWORD dwStreamNumber,
	__in      DWORD dwCallbackReason,
	__in      HANDLE hSourceFile,
	__in      HANDLE hDestinationFile,
	__in_opt  LPVOID lpData
)
{
	TRACE3( "CTaskDirScan ScanProgressRoutine r=%d n=%d t=%d\n", dwCallbackReason, dwStreamNumber, TotalBytesTransferred.LowPart );
	if ( lpData == NULL )
		return PROGRESS_CONTINUE;
	CTaskDirScan *pThis = (CTaskDirScan *)lpData;
	pThis->SetProgress( TotalBytesTransferred.QuadPart );
	if ( pThis->IsCanceled() )
		return PROGRESS_CANCEL;
	return PROGRESS_CONTINUE;
}

void CTaskDirScan::SetProgress( __int64 nDone )
{
	m_pThreadBack->SetTaskBytesDone( m_bMain, nDone ); 
}

UINT CTaskDirScan::Process(CThreadBack *pThreadBack)
{
	TRACE3( "CTaskDirScan::Process %d %d %s\n", m_nSide, m_nRecurr, m_pTreeData->GetItemNameDebug( m_posParent ) );
	m_pThreadBack = pThreadBack;
	CDocDir *pDoc = ( m_nSide == 0 ? m_pDocL : m_pDocR );
	if ( m_bForce )
		pDoc->Invalidate();
	const int &nCancel = pThreadBack->GetCancel();
	int nCancelMask = m_nSide + 1;
	BOOL bNew = pDoc->ReScanAuto( m_posParent == NULL, nCancel, nCancelMask, &ScanProgressRoutine, this );
	ASSERT( pDoc->m_bLock == FALSE );
//    TRACE1( "CTaskDirScan::Process ReScanAuto = %d\n", bNew );
	
	if ( IsCanceled() )
		return 1;

//    TRACE1( "CTaskDirScan::Process merge %s\n", pDoc->GetPathName() );
    CDocDir *pDocL = m_pDocL;
    CDocDir *pDocR = m_pDocR;
    __int64 nMaxFileSize = m_pTreeData->GetMaxFileSize(m_nSide);
    TREEPOS pos = m_pTreeData->GetFirstChildPos( m_posParent );
    if ( pos == NULL ) {        // prepare empty
        if ( m_posParent != NULL ) {
            CViewDirItem &d = m_pTreeData->GetItemData( m_posParent );
            pDoc = d.GetDoc( m_nSide );
            if ( pDoc == NULL ) {
                TRACE1( "CTaskDirScan::Process pDoc NULL %s\n", d.GetNameDebug() );
                return 1;
            }
            pDocL = d.GetDoc( 0 );
            pDocR = d.GetDoc( 1 );
        }
        ASSERT( pDoc != NULL );
    }
    else if ( m_posParent != NULL ) {
        CViewDirItem &d = m_pTreeData->GetItemData( m_posParent );
        pDoc = d.GetDoc( m_nSide );
        pDocL = d.GetDoc( 0 );
        pDocR = d.GetDoc( 1 );
        ASSERT( pDoc != NULL );
    }
	BOOL bChanged = FALSE;

	bChanged = 0 != UpdateDirect( pDoc, nMaxFileSize );
	if ( (nCancel & nCancelMask) != 0 )
		return 1;

	// update existing items
	bChanged |= 0 != UpdateItems( pos, pDoc );
	if ( (nCancel & nCancelMask) != 0 )
		return 1;

	// Check sort order / re-sort via bubble sort
	UpdateSorting( pos );
	if ( (nCancel & nCancelMask) != 0 )
		return 1;

	// insert missing items
	bChanged |= 0 != InsertNewItems( pos, pDocL, pDocR );
	if ( (nCancel & nCancelMask) != 0 )
		return 1;

	if ( bChanged ) {
		CTaskIdle *pTaskIdleUpd = CTaskIdleUpd::New( m_pTree, m_posParent );
		pThreadBack->AddIdleTask( pTaskIdleUpd );
		UpdateParentMark( m_posParent );
	}


	m_pTreeData->SetMaxFileSize(m_nSide, nMaxFileSize);

	if ( m_pTaskIdleDel != NULL )
		pThreadBack->AddIdleTask( m_pTaskIdleDel );
	m_pTaskIdleDel = NULL;
	return 0;
}

int CTaskDirScan::UpdateDirect( CDocDir *pDoc, __int64 &nMaxFileSize )
{
//    TRACE1( "CTaskDirScan::UpdateDirect %d\n", m_nSide );
	int nChanges = 0;
	CListDirEntries &deList = pDoc->GetList();
	POSITION posListDe = deList.GetHeadPosition();
	while ( posListDe != NULL && !IsCanceled() ) 
	{
		POSITION posListDeCurr = posListDe;
		CDirEntry &de = deList.GetNext( posListDe );
		if ( nMaxFileSize < de.GetFileSize() )
			nMaxFileSize = de.GetFileSize();
		POSITION posTree = de.GetViewItemPos();
		if ( posTree == NULL )
			continue;

//		TRACE1( "v@d %s\n", m_treeData.GetItemNameDebug( posTree ) );
		CViewDirItem &d = m_pTreeData->GetItemData(posTree);
		if ( de.IsDel() )
		{
			TRACE1( " Del %s\n", de.GetName() );
			ClearItemRecursive( posTree );
			++nChanges;
			CTaskIdle *pTaskIdleInv = CTaskIdleInv::New( m_pTree, posTree, FALSE );
			m_pThreadBack->AddIdleTask( pTaskIdleInv );
			continue;
		}
		else if ( de.IsMarked() )
		{
			de.SetMark( FALSE );
			++nChanges;
			CTaskIdle *pTaskIdleInv = CTaskIdleInv::New( m_pTree, posTree, FALSE );
			m_pThreadBack->AddIdleTask( pTaskIdleInv );
			if ( de.IsCopied() ) // reactivated doc
			{	// add element
//				TRACE1( " Add %s\n", de.GetName() );
				de.ResetFlags( CDirEntry::Copied );
				// CompareItem( d, pDoc, posListDe );
				d.ActivateSide( m_nSide, pDoc, posListDeCurr );
			}
			else
			{
				TRACE1( " Upd %s\n", de.GetName() );
			}
			d.UpdateMark();
		}
		else
		{
//			TRACE1( " keep %s\n", de.GetName() );
		}
	}
//	pDoc->MarkFinished();
    TRACE2( "CTaskDirScan::UpdateDirect s=%d n=%d\n", m_nSide, nChanges );
	return nChanges;
}

void CTaskDirScan::ClearItemRecursive( TREEPOS pos )
{
	CViewDirItem &d = m_pTreeData->GetItemData( pos );
	// check dependent items first
	TREEPOS posChild = m_pTreeData->GetFirstChildPos( pos );
	while ( posChild != NULL )
	{
		TREEPOS posItem = posChild;
		posChild = m_pTreeData->GetNextSiblingPos( posChild );
		ClearItemRecursive( posItem );
	}

	TRACE1( "CTaskDirScan::ClearItemRecursive %s\n", d.GetNameDebug() );
	d.ClearItem( m_nSide );

	if ( !d.IsPresent(1-m_nSide) )
	{
		d.ResetDocRef(1-m_nSide);
		if ( m_pTaskIdleDel == NULL )
			m_pTaskIdleDel = CTaskIdleDel::New( m_pTree, pos );
		else
			m_pTaskIdleDel->AddPos( pos );
	}
	return;
}

void CTaskDirScan::UpdateParentMark( TREEPOS posParent )
{
	if ( posParent == NULL )		// done
		return;

	CViewDirItem &dp = m_pTreeData->GetItemData( posParent );
	if ( dp.IsMarkDirty() )
		return;

	dp.SetMarkAll();
//  TRACE1( "CTaskDirScan::UpdateParentMark() %s\n", dp.GetName() );

	UpdateParentMark( dp.GetItemParentPos() );	// do recursive
}

int CTaskDirScan::UpdateItems( TREEPOS posChild, CDocDir *pDoc )
{
//    TRACE1( "CTaskDirScan::UpdateItems %d\n", m_nSide );
	int nChanges = 0;
	CListDirEntries &deList = pDoc->GetList();
	DOCPOS posListDe = deList.GetHeadPosition();
	while ( posListDe != NULL && !IsCanceled() ) 
	{
		DOCPOS posListDeCurr = posListDe;
		CDirEntry &de = deList.GetNext( posListDe );
		TREEPOS posTree = de.GetViewItemPos();
		if ( posTree != NULL || de.IsDel() )
			continue;

//		ASSERT( de.IsCopied() && de.IsMarked() );
		// try to locate update position
		posTree = posChild;
		while ( posTree != NULL && !IsCanceled() ) {
			const CViewDirItem &d = m_pTreeData->GetItemData( posTree );
			if ( d.Compare( m_nSide, pDoc, posListDeCurr ) == 0 )
				break;	// found
			posTree = m_pTreeData->GetNextSiblingPos( posTree );
		}
		if ( posTree != NULL ) {
//			TRACE1( " Act %s\n", de.GetName() );
			CViewDirItem &d = m_pTreeData->GetItemData( posTree );
			d.ActivateSide( m_nSide, pDoc, posListDeCurr );
			d.UpdateMark();
			de.SetViewItemPos(posTree);
			de.ResetFlags( CDirEntry::Copied | CDirEntry::Mark );
			++nChanges;
			CTaskIdle *pTaskIdleInv = CTaskIdleInv::New( m_pTree, posTree, FALSE );
			m_pThreadBack->AddIdleTask( pTaskIdleInv );
		}
	}
    TRACE2( "CTaskDirScan::UpdateItems s=%d n=%d\n", m_nSide, nChanges );
	return nChanges;
}

void CTaskDirScan::UpdateSorting( TREEPOS posStart )
{
    TRACE1( "CTaskDirScan::UpdateSorting %d\n", m_nSide );
	if ( posStart == NULL )
		return;

	BOOL bSwaped;
	do {
		bSwaped = FALSE;
		TREEPOS posChild = posStart;
		TREEPOS posPrevChild = posChild;
		posChild = m_pTreeData->GetNextSiblingPos( posChild );
		while ( posChild != NULL && !IsCanceled() ) 
		{
			const CViewDirItem &dc = m_pTreeData->GetItemData(posChild);
			if ( !dc.IsDeleted() ) {
				const CViewDirItem &dp = m_pTreeData->GetItemData(posPrevChild);
				int nComp = dc.Compare( dp, m_nSide, m_nSortType );
				ASSERT( nComp != 0 );
				if ( nComp < 0 ) {	// badly ordered: swap 
					if ( dp.GetItemHandle() == NULL ) { 
						bSwaped = TRUE;
						TRACE1( " swap %s\n", dp.GetName() );
						// verify pDoc, pos for both sides during swap - and posTree in DirEntry 
						TREEPOS posNew = m_pTreeData->CreateItem( dp, posChild );
						m_pTreeData->DeleteItem( posPrevChild );	// TODO ? or mark as deleted
						if ( dc.GetPrevSiblingPos() == NULL )
							posStart = posChild;
						posChild = posNew; // avoid 2nd compare
					}
					else
						TRACE2( " BAD %s,%s\n", dp.GetName(), dc.GetName() );
				}
				posPrevChild = posChild;
			}
			posChild = m_pTreeData->GetNextSiblingPos( posChild );
		}
//		TRACE1( " bSwaped = %d\n", bSwaped );
	} while ( bSwaped && !IsCanceled() );
}

int CTaskDirScan::InsertNewItems( TREEPOS posChild, CDocDir *pDocL, CDocDir *pDocR )
{
//    TRACE1( "CTaskDirScan::InsertNewItems %d\n", m_nSide );
	int nChanges = 0;
	CDocDir *pDoc = ( m_nSide == 0 ? pDocL : pDocR );
	CDocManFileSync *pDocMan = (CDocManFileSync *) AfxGetApp()->m_pDocManager;
	CListDirEntries &deList = pDoc->GetList();
	DOCPOS posListDe = deList.GetHeadPosition();
	while ( posListDe != NULL && !IsCanceled() ) 
	{
		DOCPOS posListDeCurr = posListDe;
		CDirEntry &de = deList.GetNext( posListDe );
		TREEPOS posTree = de.GetViewItemPos();
		if ( posTree != NULL || !de.IsCopied() )
			continue;

		++nChanges;
		ASSERT( de.IsCopied() && de.IsMarked() );

		CViewDirItem dn( m_posParent, pDocL, pDocR );
		dn.ActivateSide( m_nSide, pDoc, posListDeCurr );
		dn.UpdateMark();

		// locate insert position from behind
		TREEPOS posPrev = m_pTreeData->GetLastChildPos(m_posParent);
		while ( posPrev != NULL && !IsCanceled() ) {
			CViewDirItem &d = m_pTreeData->GetItemData( posPrev );
			if ( d.Compare( dn, m_nSide, m_nSortType ) < 0 )
				break;	// found
//			TRACE1( "  skip %s\n", d.GetNameDebug() );
			posPrev = m_pTreeData->GetPrevSiblingPos( posPrev );
		}

		TRACE2( " Ins %s %s\n", m_pTreeData->GetItemNameDebug(posPrev), de.GetName() );
		if ( dn.GetIcon() == CDocFileSync::IconUnknown )
			dn.SetIcon( pDocMan->GetMatchingIconNo( dn.GetName() ) );
		TREEPOS posNew = m_pTreeData->CreateItem( dn, posPrev );
		if ( posPrev == m_posParent )
			posChild = posNew;
		de.ResetFlags( CDirEntry::Copied | CDirEntry::Mark );
	}
    TRACE2( "CTaskDirScan::InsertNewItems s=%d n=%d\n", m_nSide, nChanges );
	return nChanges;
}

POSITION CTaskDirScan::CreateTasks( CThreadBack *pThreadBack, BOOL bMain, CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent, 
			CDocDir *pDocL, CDocDir *pDocR, int nRecurr, int nSortType, POSITION posAfter, BOOL *pbForce )
{
	BOOL b[2] = {FALSE, FALSE};
	if ( pbForce != NULL ) {
		b[0] = pbForce[0];
		b[1] = pbForce[1];
	}
	CTaskDirScan *pTask0 = NULL;
	if ( pDocL != NULL )
		pTask0 = CTaskDirScan::New( bMain, pTree, pTreeData, posParent, 0, pDocL, pDocR, nRecurr, nSortType, b[0] );

	CTaskDirScan *pTask1 = NULL;
	if ( pDocR != NULL )
		pTask1 = CTaskDirScan::New( bMain, pTree, pTreeData, posParent, 1, pDocL, pDocR, nRecurr, nSortType, b[1] );

	CTaskDirDep *pTask2 = NULL;
	if ( nRecurr != 0 ) {
		pTask2 = CTaskDirDep::New( bMain, pTree, pTreeData, posParent, nRecurr, nSortType, b );
	}

	posAfter = pThreadBack->AddTasks( pTask0, pTask1, nSortType, posAfter, pTask2 );

	CTaskDirCompFiles *pTaskDirCompFiles = CTaskDirCompFiles::New( FALSE, pTree, pTreeData, posParent );
	pThreadBack->ReplaceTask( pTaskDirCompFiles );

	TRACE0( "CTaskDirScan::CreateTasks end\n" );

	return posAfter;
}
