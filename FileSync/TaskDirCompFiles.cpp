#include "StdAfx.h"
#include "ThreadBack.h"
#include "DualTreeDir.h"
#include "DualTreeDirData.h"
#include "TaskDirScan.h"
#include "TaskIdleInv.h"
#include "resource.h"

#include "TaskDirCompFiles.h"

CStorage<CTaskDirCompFiles> CTaskDirCompFiles::s_storeTasks( 100, _T("CTaskDirCompFiles") );

CTaskDirCompFiles::CTaskDirCompFiles()
{
	m_pStore = &s_storeTasks;
}

CTaskDirCompFiles::~CTaskDirCompFiles()
{
}

CTaskDirCompFiles* CTaskDirCompFiles::New(BOOL bMain, 
			CDualTreeDir *pTree, CDualTreeDirData *pTreeData, POSITION posParent)
{
	POSITION posTask = s_storeTasks.New();
    CTaskDirCompFiles *pTask = s_storeTasks.GetPtrAt(posTask);
	pTask->m_posStorage = posTask;
	pTask->Init( bMain );
	TRACE2( "CTaskDirCompFiles::New %s c=%d\n", pTreeData->GetItemNameDebug( posParent ), s_storeTasks.GetCount() );
	pTask->m_posParent = posParent;
	pTask->m_nSide = 2;
	pTask->m_pTree = pTree;
	pTask->m_pTreeData = pTreeData;
	pTask->m_nTotal = 0;
    return pTask;
}

__int64 CTaskDirCompFiles::NewProgrBytesAll()
{
	__int64 nProgrBytesAll = 0;
	TREEPOS pos = m_pTreeData->GetFirstChildPos( m_posParent );
	while ( pos != NULL ) 
	{
		CViewDirItem &d = m_pTreeData->GetItemData( pos );
		if ( !d.HasDirIcon() )
		{
			if ( d.IsPresent(0) && d.IsPresent(1) )
			{
				nProgrBytesAll += d.GetFileSize(0) + d.GetFileSize(1);
			}
		}
		pos = m_pTreeData->GetNextSiblingPos( pos );
	}
#ifdef _DEBUG
	if ( m_nProgrBytesAll != nProgrBytesAll )
		TRACE3( "CTaskDirCompFiles::NewProgrBytesAll %s old=%I64d new=%I64d\n", m_pTreeData->GetItemNameDebug( m_posParent ), m_nProgrBytesAll, nProgrBytesAll );
#endif
	m_nProgrBytesAll = nProgrBytesAll;
	return m_nProgrBytesAll;
}


void CTaskDirCompFiles::Delete()
{
	TRACE2( "CTaskDirCompFiles::Delete %s c=%d\n", m_pTreeData->GetItemNameDebug( m_posParent ), s_storeTasks.GetCount() );
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeTasks.DeleteAt( pos );
    }
    else
        delete this;
}

BOOL CTaskDirCompFiles::IsCanceled() 
{ 
	return m_pThreadBack->IsCanceled( m_nSide ); 
}

DWORD CALLBACK CompProgressRoutine(
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
//	TRACE3( "CTaskDirCompFiles CompProgressRoutine r=%d n=%d t=%d\n", dwCallbackReason, dwStreamNumber, TotalBytesTransferred.LowPart );
	if ( lpData == NULL )
		return PROGRESS_CONTINUE;
	CTaskDirCompFiles *pThis = (CTaskDirCompFiles *)lpData;
	pThis->SetProgress( TotalBytesTransferred.QuadPart );
	if ( pThis->IsCanceled() )
		return PROGRESS_CANCEL;
	return PROGRESS_CONTINUE;
}

void CTaskDirCompFiles::SetProgress( __int64 nDone )
{
	nDone += m_nTotal;
	if ( nDone > m_nProgrBytesAll )
		nDone = m_nProgrBytesAll;
	m_pThreadBack->SetTaskBytesDone( m_bMain, nDone ); 
}

UINT CTaskDirCompFiles::Process(CThreadBack *pThreadBack)
{
	TRACE1( "CTaskDirCompFiles::Process %s\n", m_pTreeData->GetItemNameDebug( m_posParent ) );
	m_pThreadBack = pThreadBack;
//	HWND hwndMsgTarget = pThreadBack->GetHWndOwner();  
	TREEPOS pos = m_pTreeData->GetFirstChildPos( m_posParent );
//	while ( pos != NULL && !m_bCanceled ) 
	while ( pos != NULL && !pThreadBack->IsCanceled() ) 
	{
		CViewDirItem &d = m_pTreeData->GetItemData( pos );
		if ( !d.HasDirIcon() && !d.IsMarkDirty() )
			d.ResetMarkPending();
		if ( !d.HasDirIcon() && d.IsMarkDirty() )
		{
//			ASSERT( !d.IsDir() );
			if ( d.IsPresent(0) && d.IsPresent(1) )
			{
//				TRACE1( "CTaskDirCompFiles::Process file %s\n", d.GetName() );
#ifdef _DEBUG
				CString strC = d.GetName();	// debug only
#endif
				if ( d.GetDateTime(0) == d.GetDateTime(1) &&
					 d.GetFileSize(0) == d.GetFileSize(1) )
				{
					d.SetMarkEqual();
				}
				else if ( d.GetFileSize(0) == d.GetFileSize(1) && 
					d.GetFileSize(0) != 0 )
				{
					CTimeSpan h1( 0, 1, 0, 0 );
					if ( (d.GetDateTime(0) + h1) == d.GetDateTime(1) ||
						(d.GetDateTime(0) - h1) == d.GetDateTime(1) ) 
					{
						d.SetMarkEqual();		// 20130122
					}
					else
					{
						CTimeSpan dd = d.GetDateTime(1)- d.GetDateTime(0);
						LONGLONG ts = dd.GetTotalSeconds();
						if ( ts <= 2 && ts >= -2 ) {
							d.SetMarkEqual();
						}
						else {
							BOOL b = d.CompareCRC(&CompProgressRoutine, this);
							if ( pThreadBack->IsCanceled() )
								break;
							if ( b )
							{
								if ( d.HasArcIcon() && d.GetDoc(0) != NULL ) {	// 20100326 Zip with contents expanded
									return 0;
								}
								else {
									d.SetMarkEqual();
		//							UpdateParentMark( pos, pThreadBack );
								}
							} else {
								if ( !d.HasArcIcon() )
									d.SetMarkDiff();
							}
						}
					}
				}
				else if ( d.HasTextIcon() )
				{
					BOOL b =  d.CompareCRCText(&CompProgressRoutine, this);
					if ( pThreadBack->IsCanceled() )
						break;
					if ( b )
					{
						d.SetMarkEqual();
//						UpdateParentMark( pos, pThreadBack );
					} else {
						d.SetMarkDiff();
					}
				}
			}
			d.ResetMarkDirty();
//			m_bInvalidate = TRUE;
			CTaskIdle *pTaskIdle = CTaskIdleInv::New( m_pTree, pos );
			pThreadBack->ReplaceIdleTask( pTaskIdle );
			m_nTotal += d.GetFileSize(0) + d.GetFileSize(1);
		}
		pos = m_pTreeData->GetNextSiblingPos( pos );
	}

	// compare dirs

	if ( m_posParent == NULL )
		return 0;

	BOOL bEqual = TRUE;
	BOOL bSingleL = FALSE;	// 20090107
	BOOL bSingleR = FALSE;
	BOOL bPending = FALSE;
	__int64 fs[2] = { 0, 0 };

	pos = m_pTreeData->GetFirstChildPos( m_posParent );
//	while ( pos != NULL && !m_bCanceled ) 
	while ( pos != NULL && !pThreadBack->IsCanceled() ) 
	{
		CViewDirItem &d = m_pTreeData->GetItemData( pos );
//		if ( d.HasDirIcon() )
//			TRACE1( "CTaskDirCompFiles::Process sum %s\n", d.GetName() );
		if ( d.IsPresent(0) )
			fs[0] += d.GetFileSize(0);
		if ( d.IsPresent(1) )
			fs[1] += d.GetFileSize(1);

		if ( !d.IsMarkEqual() || d.IsMarkDirty() ) 		// 20100326
		{
			bEqual = FALSE;
			if ( d.IsPresent(0) != d.IsPresent(1) ) {
				bSingleL |= d.IsPresent(0);
				bSingleR |= d.IsPresent(1);
			}
			else if ( d.IsMarkSingle() ) {
				bSingleL |= d.IsMarkSingle(0);
				bSingleR |= d.IsMarkSingle(1);
			}
			else { // 090401 if ( dc.IsMarkDiff() ) {
				bSingleL = TRUE;
				bSingleR = TRUE;
			}

			if ( d.IsMarkPending() || d.IsMarkDirty() ) {
                TRACE2( "CTaskDirCompFiles::Process m=%x %s pending\n", d.GetMark(), d.GetName() );
				bPending = TRUE;
				if ( d.IsMarkDirty() && d.HasDirIcon()) {
//					TRACE2( "CTaskDirCompFiles::Process pending dirty %s m=%x\n", d.GetName(), d.GetMark() );
					//m_hIdleItem = NULL;		// restart
					//break;
					bSingleL = TRUE;
					bSingleR = TRUE;
				}
			}
//			pos = m_treeData.GetNextSiblingPos( pos );
//				}
//			}	// end while child
//
		}
		pos = m_pTreeData->GetNextSiblingPos( pos );
	}
	CViewDirItem &dp = m_pTreeData->GetItemData( m_posParent );
	int nMark = dp.GetMark();
	if ( bEqual )
		dp.SetMarkEqual();
	else if ( bSingleL && !bSingleR )
		dp.SetMarkSingle(0);
	else if ( bSingleR && !bSingleL )
		dp.SetMarkSingle(1);
	else
		dp.SetMarkDiff();

	if ( bPending )
		dp.SetMarkPending();
	else
	{
		dp.SetFileSize( 0, fs[0] );
		dp.SetFileSize( 1, fs[1] );
		if ( m_pTreeData->IncrMaxFileSize(0, fs[0]) ||
		     m_pTreeData->IncrMaxFileSize(1, fs[1]) )
			m_pTree->UpdColumnSize();
		dp.ResetMarkPending();
	}
//			if ( m_hIdleItem != NULL )
//			{
	dp.ResetMarkDirty();		// 20090128
//				m_bInvalidate = TRUE;
	if ( nMark != dp.GetMark() ) {
		CTaskIdle *pTaskIdle = CTaskIdleInv::New( m_pTree, m_posParent );
		pThreadBack->ReplaceIdleTask( pTaskIdle );
		UpdateParentMark( m_posParent, pThreadBack );
	}
//			}
	return 0;
}

void CTaskDirCompFiles::UpdateParentMark( TREEPOS pos, CThreadBack *pThreadBack )
{
	TREEPOS posParent = m_pTreeData->GetItemData( pos ).GetItemParentPos();
	if ( posParent == NULL )		// done
		return;

	CViewDirItem &dp = m_pTreeData->GetItemData( posParent );
//	if ( dp.IsMarkDirty() )
//		return;

	dp.SetMarkAll();
	TRACE1( "CTaskDirCompFiles::UpdateParentMark() %s\n", dp.GetName() );

	CTaskBase *pTaskDirCompFiles = CTaskDirCompFiles::New( m_bMain, m_pTree, m_pTreeData, posParent );
	pThreadBack->ReplaceTask( pTaskDirCompFiles );
	CTaskIdle *pTaskIdle = CTaskIdleInv::New( m_pTree, posParent );
	pThreadBack->ReplaceIdleTask( pTaskIdle );

	UpdateParentMark( posParent, pThreadBack );	// do recursive
}

int CTaskDirCompFiles::Compare( CTaskBase *pTask )
{
	CTaskDirCompFiles* pTaskOther = (CTaskDirCompFiles*) pTask;
	if ( pTaskOther->m_pStore != m_pStore )
		return 1;
	if ( pTaskOther->m_posParent != m_posParent )
		return 1;
	if ( pTaskOther->m_bMain != m_bMain )
		return 1;
	TRACE2("CTaskDirCompFiles::Compare match %s %d\n", m_pTree->GetItemNameDebug(m_posParent), m_bMain );
	return 0;
}
