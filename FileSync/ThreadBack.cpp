#include "StdAfx.h"
#include "resource.h"
#include "TaskIdle.h"
#include "TaskIdleInv.h"
#include "TaskIdleDel.h"
#include "TaskIdleExp.h"
#include "TaskIdleUpd.h"
#include "TaskDirScan.h"
#include "TaskDirDep.h"
#include "TaskDirExp.h"
#include "TaskDirCompFiles.h"

#include "ThreadBack.h"


CThreadBack::CThreadBack(void) :
	m_listTaskMain(100), m_listTaskOpt(100), m_listTaskIdle(100)
{
	m_pThread = NULL;
	m_posTaskMain = NULL;
	m_posTaskOpt = NULL;
	m_posTaskIdle = NULL;
	m_pEventCont = new CEvent( FALSE, TRUE );		// continue background thread
	m_pEventStop = new CEvent( FALSE, TRUE );
	m_pTaskCurr = NULL;
//	m_bCanceled = FALSE;
	m_nCancel = 0;
	m_bCleanup = FALSE;
	m_nProgressTasks = 0;
	m_nProgressDone = 0;
	m_nProgrBytesAll[0] = 0;
	m_nProgrBytesAll[1] = 0;
	m_nProgrBytesDone[0] = 0;
	m_nProgrBytesDone[1] = 0;
	m_nTaskBytesDone[0] = 0;
	m_nTaskBytesDone[1] = 0;
	m_hwndOwner = NULL;
	m_nIdleMax = 0;
	m_pTaskReady = NULL;
	m_bRecalc = FALSE;
}


CThreadBack::~CThreadBack(void)
{
	Finish();
	delete m_pEventCont;
	delete m_pEventStop;
}

UINT __cdecl CThreadBack::MyThreadProc( LPVOID pParam )
{
    CThreadBack* pThis = (CThreadBack*)pParam;

    if (pThis == NULL)
		return 1;   // bad param

	CSyncObject* pWaitObjects[] = { pThis->m_pEventStop, pThis->m_pEventCont };
	CMultiLock multiLock( pWaitObjects, 2L );

	while ( TRUE )
	{
		CTaskBase* pTaskCurr = pThis->FetchNextTask();
		if ( pTaskCurr != NULL ) {
			CSingleLock singleLockTaskBusy(&(pThis->m_CSTaskBusy), TRUE);
			pTaskCurr = pThis->m_pTaskCurr;
			UINT nRC = 0;
			if ( pTaskCurr != NULL ) {
				AssertX(AfxIsValidAddress(pTaskCurr, sizeof(CTaskBase)));
				AssertX(AfxIsValidAddress(*(void**)pTaskCurr, sizeof(void*), FALSE));
				nRC = pTaskCurr->Process(pThis);
				BOOL bMain = pTaskCurr->IsMainTask();
				pThis->m_nProgrBytesDone[bMain] += pTaskCurr->GetProgrBytesAll();
				pThis->m_nTaskBytesDone[bMain] = 0;
			}
#ifdef _DEBUG
			if ( nRC == 1 ) {
				TRACE0( "CThreadBack::MyThreadProc() task canceled\n" );
			}
#endif
			pThis->m_nProgressDone += 100;
		}
		else
		{	// no current task
			pThis->m_nProgressDone = 100;
//			pThis->m_pEventWait->SetEvent();
			TRACE0( "CThreadBack::MyThreadProc() wait\n" );
			PostMessage( pThis->m_hwndOwner, WM_USER_STARTIDLE, 0, 0 );		// dummy to restart idle processing
			DWORD dwLock = multiLock.Lock( INFINITE, FALSE );		// wait for Stop or Cont
			if ( dwLock == WAIT_OBJECT_0 ) // EventStop
				break;
//			pThis->m_pEventCont->ResetEvent();
//			pThis->m_pEventWait->ResetEvent();
			TRACE0( "CThreadBack::MyThreadProc() cont\n" );
		}
	}
    return 0;   // thread completed successfully
}

CTaskBase* CThreadBack::FetchNextTask()
{
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);

	m_pTaskCurr = NULL;
	m_nTaskBytesDone[0] = 0;
	m_nTaskBytesDone[1] = 0;
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::FetchNextTask() Canceled\n" );
		//RemoveAllTasks();
		m_bCleanup = TRUE;
		m_nProgressTasks = 0;
		m_nProgrBytesAll[0] = 0;
		m_nProgrBytesAll[1] = 0;
		m_nProgrBytesDone[0] = 0;
		m_nProgrBytesDone[1] = 0;
		m_pEventCont->ResetEvent();
		return NULL;
	}

	// try main
	if ( m_posTaskMain == NULL ) {
		POSITION pos = m_listTaskMain.GetHeadPosition();
		if ( pos != NULL ) {
			m_posTaskMain = pos;
			m_pTaskCurr = m_listTaskMain.GetAt( m_posTaskMain );
			TRACE0( "CThreadBack::FetchNextTask() Main Head\n" );
			m_bRecalc = TRUE;
			return m_pTaskCurr;
		}
	}
	else 
	{
		POSITION pos = m_posTaskMain;
		m_listTaskMain.GetNext( pos );
		if ( pos != NULL ) {
			m_posTaskMain = pos;
			m_pTaskCurr = m_listTaskMain.GetAt( pos );
			TRACE0( "CThreadBack::FetchNextTask() Main\n" );
			m_bRecalc = TRUE;
			return m_pTaskCurr;
		}
	}
	// try opt
	Sleep(1);
	if ( m_pTaskReady != NULL ) {	// 20120601 
		AddIdleTask( m_pTaskReady );
		m_pTaskReady = NULL;
	}

	if ( m_bRecalc ) {
		m_bRecalc = FALSE;
		RecalcOpt( NULL );
	}

	if ( m_posTaskOpt == NULL ) {
		POSITION pos = m_listTaskOpt.GetHeadPosition();
		if ( pos != NULL ) {
			m_posTaskOpt = pos;
			m_pTaskCurr = m_listTaskOpt.GetAt( m_posTaskOpt );
			TRACE0( "CThreadBack::FetchNextTask() Opt Head\n" );
			// recalc size
			RecalcOpt( pos );
			return m_pTaskCurr;
		}
	}
	else 
	{
		POSITION pos = m_posTaskOpt;
		m_listTaskOpt.GetNext( pos );
		if ( pos != NULL ) {
			m_posTaskOpt = pos;
			m_pTaskCurr = m_listTaskOpt.GetAt( pos );
			TRACE0( "CThreadBack::FetchNextTask() Opt\n" );
			// recalc size
			RecalcOpt( pos );
			return m_pTaskCurr;
		}
	}
	TRACE0( "CThreadBack::FetchNextTask() Cleanup\n" );
//	RemoveFinishedTasks();
	//RemoveAllTasks();
	m_bCleanup = TRUE;
	m_nProgressTasks = 0;
	m_nProgrBytesAll[0] = 0;
	m_nProgrBytesAll[1] = 0;
	m_nProgrBytesDone[0] = 0;
	m_nProgrBytesDone[1] = 0;
	m_nTaskBytesDone[0] = 0;
	m_nTaskBytesDone[1] = 0;
	m_pEventCont->ResetEvent();
	return NULL;
}

void CThreadBack::RecalcOpt( POSITION pos )
{
	__int64 nProgrBytesAll = 0;
	if ( pos == NULL ) {
		pos = m_listTaskOpt.GetHeadPosition();
		while ( pos != NULL ) {
			CTaskBase* pTask = m_listTaskOpt.GetNext( pos );
			nProgrBytesAll += pTask->NewProgrBytesAll();
		}
	}
	else {	// update
		nProgrBytesAll = m_nProgrBytesAll[0];
		while ( pos != NULL ) {
			CTaskBase* pTask = m_listTaskOpt.GetNext( pos );
			nProgrBytesAll -= pTask->GetProgrBytesAll();
			nProgrBytesAll += pTask->NewProgrBytesAll();
		}
	}
	m_nProgrBytesAll[0] = nProgrBytesAll;
}

void CThreadBack::RemoveAllTasks()
{
	// should be already locked:
//	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);

	TRACE0( "CThreadBack::RemoveAllTasks()\n" );
	m_pTaskCurr = NULL;

	m_posTaskMain = m_listTaskMain.GetHeadPosition();
	while ( m_posTaskMain != NULL ) {
		m_listTaskMain.GetNext( m_posTaskMain )->Delete();
	}
	m_listTaskMain.RemoveAll();

	m_posTaskOpt = m_listTaskOpt.GetHeadPosition();
	while ( m_posTaskOpt != NULL ) {
		m_listTaskOpt.GetNext( m_posTaskOpt )->Delete();
	}
	m_listTaskOpt.RemoveAll();

	m_nProgressTasks = 0;
	m_nProgrBytesAll[0] = 0;
	m_nProgrBytesAll[1] = 0;
	m_nProgrBytesDone[0] = 0;
	m_nProgrBytesDone[1] = 0;
	m_nTaskBytesDone[0] = 0;
	m_nTaskBytesDone[1] = 0;
	m_bCleanup = FALSE;	// done
}

void CThreadBack::RemoveAllIdleTasks()
{
	CSingleLock singleLockTaskList(&m_CSTaskIdleList, TRUE);
	TRACE0( "CThreadBack::RemoveAllIdleTasks()\n" );
	m_posTaskIdle = m_listTaskIdle.GetHeadPosition();
	while ( m_posTaskIdle != NULL ) {
		m_listTaskIdle.GetNext( m_posTaskIdle )->Delete();
	}
	m_listTaskIdle.RemoveAll();
	m_nIdleMax = 0;

	if ( m_pTaskReady != NULL ) {
		m_pTaskReady->Delete();
		m_pTaskReady = NULL;
	}
	CTaskIdleDel::AssertEmpty();
	CTaskIdleExp::AssertEmpty();
	CTaskIdleInv::AssertEmpty();
	CTaskIdleUpd::AssertEmpty();
	TRACE0( "CThreadBack::RemoveAllIdleTasks() done\n" );
//	ASSERT( CTaskIdleInv::GetStoreTaskCount() == 0 );
}

void CThreadBack::RemoveAllTasksSide( int nSide )
{
	// remove all finished tasks and unfinished ones only for the given side (2 == common)
//	if (m_pTaskCurr != NULL && (m_pTaskCurr->m_nSide == nSide || m_pTaskCurr->m_nSide == 2 ) )
//		m_pTaskCurr = NULL;
//	ASSERT( m_pTaskCurr == NULL || m_pTaskCurr->m_nSide != nSide) ;

	POSITION posTaskMain = m_listTaskMain.GetHeadPosition();
	BOOL bDone = (m_posTaskMain != NULL);
	while ( posTaskMain != NULL ) {
		POSITION pos = posTaskMain;
		if ( pos == m_posTaskMain )
			bDone = FALSE;
		CTaskBase* pTask = m_listTaskMain.GetNext( posTaskMain );
		if ( bDone || pTask->m_nSide == nSide || pTask->m_nSide == 2) {
			if ( pTask == m_pTaskCurr )
				m_pTaskCurr = NULL;
			m_nProgrBytesAll[1] -= pTask->GetProgrBytesAll();
			pTask->Delete();
			m_listTaskMain.RemoveAt( pos );
			--m_nProgressTasks;
			if ( pos == m_posTaskMain )
				m_posTaskMain = NULL;
		}
	}

	POSITION posTaskOpt = m_listTaskOpt.GetHeadPosition();
	bDone = (m_posTaskOpt != NULL);
	while ( posTaskOpt != NULL ) {
		POSITION pos = posTaskOpt;
		if ( pos == m_posTaskOpt )
			bDone = FALSE;
		CTaskBase* pTask = m_listTaskOpt.GetNext( posTaskOpt );
		if ( bDone || pTask->m_nSide == nSide || pTask->m_nSide == 2 ) {
			if ( pTask == m_pTaskCurr )
				m_pTaskCurr = NULL;; 
			m_nProgrBytesAll[0] -= pTask->GetProgrBytesAll();
			pTask->Delete();
			m_listTaskOpt.RemoveAt( pos );
			--m_nProgressTasks;
			if ( pos == m_posTaskOpt )
				m_posTaskOpt = NULL;
		}
	}
}

//void CThreadBack::RemoveFinishedTasks()
//{
//}

POSITION CThreadBack::AddTask(CTaskBase *pTask, POSITION posAfter)
{
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::AddTask ign.\n" );
		pTask->Delete();
		return NULL;
	}
	BOOL bMain = pTask->IsMainTask();
	int nSide = pTask->GetSide();
	POSITION pos = NULL;
	TRACE2( "CThreadBack::AddTask s=%d m=%d\n", nSide, bMain );
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);
	if ( m_bCleanup )
		RemoveAllTasks();
	if ( bMain ) {
		pos = m_listTaskMain.InsertAfter(posAfter, pTask);
	} else {
		pos = m_listTaskOpt.InsertAfter(posAfter, pTask);
	}
	++m_nProgressTasks;
	m_nProgrBytesAll[bMain] += pTask->GetProgrBytesAll();
	if ( m_pTaskCurr == NULL ) {
		TRACE0( "CThreadBack::AddTask cont.\n" );
		m_pEventCont->SetEvent();
	}
	return pos;
}

POSITION CThreadBack::AddTask(CTaskBase *pTask, CTaskBase *pCurrTask)
{
	BOOL bMain = pCurrTask->IsMainTask();
	ASSERT( pTask->IsMainTask() == bMain );
	POSITION posAfter = ( bMain ? m_posTaskMain : m_posTaskOpt );
	return AddTask( pTask, posAfter );
}

POSITION CThreadBack::ReplaceTask(CTaskBase *pTask)
{
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::ReplaceTask ign.\n" );
		pTask->Delete();
		return NULL;
	}
	BOOL bMain = pTask->IsMainTask();
	int nSide = pTask->GetSide();
	POSITION pos = NULL;
//	TRACE1( "CThreadBack::AddTask %d\n", nSide );
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);
	if ( m_bCleanup )
		RemoveAllTasks();
	if ( bMain ) {
		pos = m_listTaskMain.InsertAfter(NULL, pTask);
		++m_nProgressTasks;
		m_nProgrBytesAll[1] += pTask->GetProgrBytesAll();
		POSITION posPrev = pos;
		m_listTaskMain.GetPrev(posPrev);
		while ( posPrev != NULL && posPrev != m_posTaskMain ) {
			CTaskBase *pTaskPrev = m_listTaskMain.GetAt(posPrev);
			if ( pTaskPrev->Compare(pTask) == 0 ) {
				--m_nProgressTasks;
				m_nProgrBytesAll[1] -= pTask->GetProgrBytesAll();
				m_listTaskMain.RemoveAt(posPrev);
				pTaskPrev->Delete();
				break;
			}
			m_listTaskMain.GetPrev(posPrev);
		}
	} else {
		pos = m_listTaskOpt.InsertAfter(NULL, pTask);
		++m_nProgressTasks;
		m_nProgrBytesAll[0] += pTask->GetProgrBytesAll();
		POSITION posPrev = pos;
		m_listTaskOpt.GetPrev(posPrev);
		while ( posPrev != NULL && posPrev != m_posTaskOpt ) {
			CTaskBase *pTaskPrev = m_listTaskOpt.GetAt(posPrev);
			if ( pTaskPrev->Compare(pTask) == 0 ) {
				--m_nProgressTasks;
				m_nProgrBytesAll[0] -= pTaskPrev->GetProgrBytesAll();
				m_listTaskOpt.RemoveAt(posPrev);
				pTaskPrev->Delete();
				break;
			}
			m_listTaskOpt.GetPrev(posPrev);
		}
	}
//	singleLockTaskList.Unlock();
	if ( m_pTaskCurr == NULL ) {
		TRACE0( "CThreadBack::ReplaceTask cont.\n" );
//		m_pEventWait->ResetEvent();
		m_pEventCont->SetEvent();
	}
	return pos;
}

POSITION CThreadBack::AddTasks(CTaskBase *pTask0, CTaskBase *pTask1, int nSortType, POSITION posAfter, CTaskBase *pTask2)
{
	ASSERT( pTask0 == NULL || pTask1 == NULL || pTask0->IsMainTask() == pTask1->IsMainTask() );
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::AddTasks ign.\n" );
		if ( pTask0 != NULL ) pTask0->Delete();
		if ( pTask1 != NULL ) pTask1->Delete();
		if ( pTask2 != NULL ) pTask2->Delete();
		return NULL;
	}
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);
	POSITION pos = NULL;
	if ( nSortType >= ID_SORT_RIGHT_SIZE ) {
		if ( pTask1 != NULL ) pos = AddTask( pTask1, posAfter );
		if ( pos != NULL ) posAfter = pos;
		if ( pTask0 != NULL ) pos = AddTask( pTask0, posAfter );
	} else {
		if ( pTask0 != NULL ) pos = AddTask( pTask0, posAfter );
		if ( pos != NULL ) posAfter = pos;
		if ( pTask1 != NULL ) pos = AddTask( pTask1, posAfter );
	}
	if ( pos != NULL ) posAfter = pos;
	if ( pTask2 != NULL ) pos = AddTask( pTask2, posAfter );
	return pos;
}

POSITION CThreadBack::AddIdleTask(CTaskIdle *pTask)
{
//	TRACE0( "CThreadBack::AddIdleTask\n" );
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::AddIdleTask() Canceled\n" );
		pTask->Delete();
		return NULL;
	}
	CSingleLock singleLockTaskList(&m_CSTaskIdleList, TRUE);
	POSITION pos = m_listTaskIdle.AddTail( pTask );
	++m_nIdleMax;
	return pos;
}

POSITION CThreadBack::ReplaceIdleTask(CTaskIdle *pTask)
{
//	TRACE0( "CThreadBack::ReplaceIdleTask\n" );
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::ReplaceIdleTask() Canceled\n" );
		pTask->Delete();
		return NULL;
	}
	CSingleLock singleLockTaskList(&m_CSTaskIdleList, TRUE);
	POSITION pos = m_listTaskIdle.AddTail( pTask );
	++m_nIdleMax;

	POSITION posPrev = pos;
	m_listTaskIdle.GetPrev(posPrev);
	while ( posPrev != NULL && posPrev != m_posTaskIdle ) {
		CTaskIdle *pTaskPrev = m_listTaskIdle.GetAt(posPrev);
		if ( pTaskPrev->Compare(pTask) == 0 ) {
			--m_nIdleMax;
			m_listTaskIdle.RemoveAt(posPrev);
			pTaskPrev->Delete();
			break;
		}
		m_listTaskIdle.GetPrev(posPrev);
	}
	return pos;
}

void CThreadBack::AddReadyTask( CTaskIdle *pTask )
{ 
	TRACE0( "CThreadBack::AddReadyTask()\n" );
	if ( m_pTaskReady != NULL ) {
		m_pTaskReady->Delete();
	}
	m_pTaskReady = pTask; 
}

BOOL CThreadBack::ProcessIdleStep()
{
//	if ( m_bCanceled ) {
	if ( m_nCancel != 0 ) {
		TRACE0( "CThreadBack::ProcessIdleStep() Canceled\n" );
		return FALSE;
	}
	CSingleLock singleLockTaskList(&m_CSTaskIdleList, TRUE);
	if ( m_posTaskIdle == NULL ) {
		m_posTaskIdle = m_listTaskIdle.GetHeadPosition();
		if ( m_posTaskIdle == NULL ) {	// queue empty
//			if ( m_pTaskReady != NULL && IsReady() ) {			// moved 20120601
//				TRACE0( "CThreadBack::ProcessIdleStep() TaskReady\n" );
//				AssertX(AfxIsValidAddress(m_pTaskReady, sizeof(CTaskIdle)));
//				AssertX(AfxIsValidAddress(*(void**)m_pTaskReady, sizeof(void*), FALSE));
//				int nRet = m_pTaskReady->ProcessStep();
//				m_pTaskReady->Delete();
//				m_pTaskReady = NULL;
//			}
			return FALSE;	// no more idle steps
		}
//		TRACE0( "CThreadBack::ProcessIdleStep() Started Head\n" );
	}

	CTaskIdle *pTask = m_listTaskIdle.GetAt(m_posTaskIdle);
	AssertX(AfxIsValidAddress(pTask, sizeof(CTaskIdle)));
	AssertX(AfxIsValidAddress(*(void**)pTask, sizeof(void*), FALSE));
	int nRet = pTask->ProcessStep();

	if ( nRet == 1 )
		return TRUE;	// continue with next step from same task

#ifdef _DEBUG
	if ( nRet != 0 ) {
		TRACE0( "CThreadBack::ProcessIdleStep() failed #################\n" );
	}
#endif

//	CSingleLock singleLockTaskList(&m_CSTaskIdleList, TRUE);
	POSITION posOld = m_posTaskIdle;
	m_listTaskIdle.GetNext( m_posTaskIdle );
	pTask->Delete();
	m_listTaskIdle.RemoveAt( posOld );
	if ( m_listTaskIdle.IsEmpty() )
		m_nIdleMax = 0;
//	TRACE1( "CThreadBack::ProcessIdleStep() task finished (%d remaining)\n", m_listTaskIdle.GetCount() );
	return TRUE;	// continue with next task
}

void CThreadBack::Start(HWND hwndOwner)
{
	m_hwndOwner = hwndOwner;
	m_pThread = AfxBeginThread(MyThreadProc, this, THREAD_PRIORITY_NORMAL, 0U, CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}

void CThreadBack::CancelAll()
{
	TRACE0( "CThreadBack::CancelAll\n" );
//	m_bCanceled = TRUE;
	m_nCancel = 3;	// both

//	if ( m_pTaskCurr != NULL ) {
//		TRACE0( "CThreadBack::CancelAll stop task\n" );
//		m_pTaskCurr->m_bCanceled = TRUE;
//	}
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);
	CSingleLock singleLockTaskBusy(&m_CSTaskBusy, TRUE);
//	ASSERT( m_pTaskCurr == NULL) ;
	RemoveAllTasks();
	RemoveAllIdleTasks();
	m_pTaskReady = NULL;
//	m_bCanceled = FALSE;
	m_nCancel = 0;

	CTaskDirScan::AssertEmpty();
	CTaskDirDep::AssertEmpty();
	CTaskDirExp::AssertEmpty();
	CTaskDirCompFiles::AssertEmpty();
	TRACE0( "CThreadBack::CancelAll done\n" );
}

void CThreadBack::CancelSide( int nSide )
{
	TRACE1( "CThreadBack::CancelSide %d\n", nSide );
//	m_bCanceled = TRUE;
	m_nCancel = nSide + 1;
	CSingleLock singleLockTaskList(&m_CSTaskList, TRUE);

//	if ( m_pTaskCurr != NULL && 
//		(m_pTaskCurr->m_nSide == nSide || m_pTaskCurr->m_nSide == 2) ) {
//		TRACE0( "CThreadBack::CancelSide stop task\n" );
//		m_pTaskCurr->m_bCanceled = TRUE;
//		TRACE0( "CThreadBack::CancelSide lockWait\n" );
//		CSingleLock lockWait(m_pEventWait);
//		lockWait.Lock();	// wait for task finished
//	}
	CSingleLock singleLockTaskBusy(&m_CSTaskBusy, TRUE);
	RemoveAllTasksSide( nSide );
	RemoveAllIdleTasks();
	m_pTaskReady = NULL;
//	m_bCanceled = FALSE;
	m_nCancel = 0;
}

void CThreadBack::Finish()
{
	ASSERT(m_pThread != NULL);
	CancelAll();
	m_pEventStop->SetEvent();
	WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	delete m_pThread;
	m_pThread = NULL;
}

int CThreadBack::GetProgress()
{
	if ( m_nProgressTasks < 1 )
		return 100;
	//return m_nProgressDone / m_nProgressTasks;
	int p = (int)( ( m_nProgrBytesDone[0] + m_nTaskBytesDone[0] + m_nProgrBytesDone[1] + m_nTaskBytesDone[1] ) * 100 / 
		( m_nProgrBytesAll[0] + m_nProgrBytesAll[1] ) );
	if ( p > 100 ) {
		m_bRecalc = TRUE;
		p = 100;
	}
	return p;
}

int CThreadBack::GetProgressIdle()
{
	if ( m_nIdleMax == 0 )
		return 100;
	return 100 * (m_nIdleMax - m_listTaskIdle.GetCount()) / m_nIdleMax;
}

CString Fmt64( __int64 n )
{
	CString str;
	if ( n < 0 )
		str = _T("0");
	else if ( n < 1000 )
		str.Format( _T("%I64d"), n );
	else if ( n < 100000 )
		str.Format( _T("%I64d.%I64dK"), n / 1000, n % 1000 / 100 );
	else if ( n < 1000000 )
		str.Format( _T("%I64dK"), n / 1000 );
	else if ( n < 100000000 )
		str.Format( _T("%I64d.%I64dM"), n / 1000000, n % 1000000 / 100000 );
	else if ( n < 1000000000 )
		str.Format( _T("%I64dM"), n / 1000000 );
	else
		str.Format( _T("%I64dG"), n / 1000000000 );
	return str;
}

CString CThreadBack::GetProgressString()
{
	CString str;
//	str.Format( _T("% 3d%% / % 3d%% done"), GetProgress(), GetProgressIdle() );
//	str.Format( _T("% 3d%% done"), GetProgress() );
	__int64 nDone[2];
	nDone[0] = m_nProgrBytesDone[0] + m_nTaskBytesDone[0];
	nDone[1] = m_nProgrBytesDone[1] + m_nTaskBytesDone[1];
//	if ( nDone[0] > m_nProgrBytesAll[0] )
//		nDone[0] = m_nProgrBytesAll[0];
	str.Format( _T("% 3d%% done (%s / %s) (%s / %s)"), GetProgress(), 
		Fmt64(nDone[1]), Fmt64(m_nProgrBytesAll[1]), Fmt64(nDone[0]), Fmt64(m_nProgrBytesAll[0]) );
	return str;
}

BOOL CThreadBack::IsReadyOpt() const
{
	if ( m_bCleanup )
		return TRUE;
	if ( m_listTaskMain.GetHeadPosition() == NULL )
		return TRUE;
	if ( m_listTaskMain.GetTailPosition() == m_posTaskMain )
		return TRUE;
	return FALSE;
}
