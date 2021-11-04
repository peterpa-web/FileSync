#pragma once
#include "afxmt.h"
#include "TaskBase.h"
#include "TaskIdle.h"

#define WM_USER_STARTIDLE  (WM_APP + 1)


typedef CTypedPtrList<CPtrList,CTaskBase*> CTaskList;
typedef CTypedPtrList<CPtrList,CTaskIdle*> CTaskIdleList;

class CTaskIdle;

class CThreadBack
{
public:
	CThreadBack(void);
	~CThreadBack(void);

	POSITION AddTask(CTaskBase *pTask, POSITION posAfter = NULL);
	POSITION AddTask(CTaskBase *pTask, CTaskBase *pCurrTask);
	POSITION ReplaceTask(CTaskBase *pTask);
	POSITION AddTasks(CTaskBase *pTask0, CTaskBase *pTask1, int nSortType, POSITION posAfter = NULL, CTaskBase *pTask2 = NULL);
	POSITION AddIdleTask( CTaskIdle *pTask );
	POSITION ReplaceIdleTask( CTaskIdle *pTask );
	void AddReadyTask( CTaskIdle *pTask );
	BOOL ProcessIdleStep();
	void Start(HWND hwndOwner);
	void CancelAll();
	void CancelSide( int nSide );
	void Finish();
	HWND GetHWndOwner() { return m_hwndOwner; }
	int GetProgress();
	int GetProgressIdle();
	CString GetProgressString(); // backthread + idle
	POSITION GetTaskPos( BOOL bMain ) const { return ( bMain ? m_posTaskMain : m_posTaskOpt ); }
//	BOOL IsReadyOpt() const { return m_bCleanup || m_listTaskMain.GetHeadPosition() == NULL || m_listTaskMain.GetTailPosition() == m_posTaskMain; }
	BOOL IsReadyOpt() const;
	BOOL IsReady() const { return m_bCleanup || ((m_listTaskMain.GetHeadPosition() == NULL) && 
		m_listTaskOpt.GetHeadPosition() == NULL && m_nProgressTasks == 0); }
//	BOOL NeedCleanup() const { return m_bCleanup; }
	BOOL IsReadyIdle() const { return m_listTaskIdle.IsEmpty(); }
	BOOL IsCanceled() const { return m_nCancel != 0; }
	BOOL IsCanceled( int nSide ) const { return ((m_nCancel & (nSide + 1)) != 0); }
	const int &GetCancel() const { return m_nCancel; }
	void SetTaskBytesDone( BOOL bMain, __int64 nTaskBytesDone ) { m_nTaskBytesDone[bMain] = nTaskBytesDone; }
	
private:
	HWND m_hwndOwner;
	CWinThread* m_pThread;
	CCriticalSection m_CSTaskList;
	CCriticalSection m_CSTaskBusy;
	CCriticalSection m_CSTaskIdleList;
//	CEvent *m_pEventWait;
	CEvent *m_pEventCont;
	CEvent *m_pEventStop;
	CTaskList m_listTaskMain;
	CTaskList m_listTaskOpt;
	CTaskIdleList m_listTaskIdle;
	CTaskIdle *m_pTaskReady;
	POSITION m_posTaskMain;
	POSITION m_posTaskOpt;
	POSITION m_posTaskIdle;
	CTaskBase* m_pTaskCurr;
//	BOOL m_bCanceled;
	int m_nCancel;		// 1=left 2=right 3=both
	BOOL m_bCleanup;
	int m_nProgressTasks;
	int m_nProgressDone;	// task count * 100
	__int64 m_nProgrBytesAll[2];
	__int64 m_nProgrBytesDone[2];
	__int64 m_nTaskBytesDone[2];
	int m_nIdleMax; // for ProgressIdle
	BOOL m_bRecalc;

	static UINT __cdecl MyThreadProc( LPVOID pParam );
	CTaskBase* FetchNextTask();

	void RecalcOpt( POSITION pos );
	void RemoveAllTasks();
	void RemoveAllIdleTasks();
	void RemoveAllTasksSide(int nSide);
//	void RemoveFinishedTasks();
};

