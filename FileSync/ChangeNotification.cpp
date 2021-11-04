#include "StdAfx.h"
#include "sys\stat.h"

#include "ChangeNotification.h"

// alternative: ReadDirectoryChangesW, CancelIo, CloseHandle

HANDLE					CChangeNotification::s_hChangeHandles[MAX_CHANGE_HANDLES+1] = { NULL };
CChangeNotification*	CChangeNotification::s_pObj[MAX_CHANGE_HANDLES+1];
int						CChangeNotification::s_nHandles = 1;	// first reserved for event
CChangeNotification*	CChangeNotification::s_pObjRst[MAX_CHANGE_HANDLES];
int						CChangeNotification::s_nHandlesRst = 0;	
CWinThread*				CChangeNotification::s_pThread = NULL;
CChangeNotification::Intr CChangeNotification::s_intr = None;
CCriticalSection		CChangeNotification::s_CS;

CChangeNotification::CChangeNotification(void)
{
	m_hChangeHandle = INVALID_HANDLE_VALUE;
	m_bEnabled = TRUE;
	m_lChanged = 0;
	m_bRestart = FALSE;
}

CChangeNotification::~CChangeNotification(void)
{
	MonitorEnd();
}

BOOL CChangeNotification::MonitorDir(
		LPCTSTR lpPathName,    // directory name
		BOOL bWatchSubtree,    // monitoring option
		DWORD dwNotifyFilter   // filter conditions
	)
{
	TRACE1("CChangeNotification::MonitorDir %s\n", lpPathName);
	if ( !MonitorEnd() )
		return FALSE;
	m_strPath = lpPathName;
	m_strFile.Empty();
	m_bWatchSubtree = bWatchSubtree;
	m_dwNotifyFilter = dwNotifyFilter;
	if ( m_strPath.IsEmpty() )
		return FALSE;
	m_bEnabled = TRUE;
	return MonitorStart();
}

BOOL CChangeNotification::MonitorFile(
		LPCTSTR lpPathName,    // directory name
		LPCTSTR lpFileName     // file name
	)
{
	TRACE1("CChangeNotification::MonitorFile %s\n", lpFileName );
	if ( !MonitorEnd() )
		return FALSE;
	m_strPath = lpPathName;
	m_strFile = lpFileName;
	m_bWatchSubtree = FALSE;
	m_dwNotifyFilter = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES;
	int result = _tstati64( m_strFile, &m_fs );
	if ( result != 0 )
		return FALSE;
	m_bEnabled = TRUE;
	return MonitorStart();
}

BOOL CChangeNotification::MonitorEnd()
{
	TRACE2("CChangeNotification::MonitorEnd %s %s\n", m_strPath, m_strFile );
	if ( !Stop( this ) )
		return FALSE;

	if ( m_hChangeHandle != INVALID_HANDLE_VALUE && m_hChangeHandle != NULL )
	{
		FindCloseChangeNotification( m_hChangeHandle );
		m_hChangeHandle = INVALID_HANDLE_VALUE;
	}
	m_lChanged = 0;
	m_bRestart = FALSE;
	return TRUE;
}

void CChangeNotification::Enable( BOOL bEnable /* TRUE */ )
{
//	TRACE3("CChangeNotification::Enable %d %s %s\n", bEnable, m_strPath, m_strFile );
	Sleep( 0 );
	if ( m_bEnabled != bEnable )
	{
		if ( bEnable )
		{
			m_bEnabled = MonitorStart();
		}
		else
		{
			m_bEnabled = !MonitorEnd();
		}
	}
}

void CChangeNotification::Signal()
{
	InterlockedExchange( &m_lChanged, 1 ); 
}


LONG CChangeNotification::WhatIsChanged()
{
	if ( m_lChanged == 0 )
		return 0;
	LONG lChanged = InterlockedExchange( &m_lChanged, 0 );
	return lChanged;
}

void CChangeNotification::Cleanup()
{
	TRACE0("CChangeNotification::Cleanup\n");
	if ( s_pThread == NULL )
		return;

	ASSERT ( s_intr == None );
	SetEvent( s_hChangeHandles[0] );
	CSingleLock singleLock(&s_CS, TRUE);
	ASSERT ( s_intr == None );
	ASSERT ( s_nHandles == 1 ); // 1 is required, additional handles should be removed before
	ASSERT ( s_nHandlesRst == 0 );
	s_intr = Exit;
	SetEvent( s_hChangeHandles[0] );
	singleLock.Unlock();

	DWORD dwWait = WaitForSingleObject( s_pThread->m_hThread, 1000 );
	if ( dwWait != WAIT_OBJECT_0 )
	{
		TRACE0("Cleanup: Thread not finished after 1s\n");
		ASSERT( CheckWait( dwWait ) );
	}
	CloseHandle( s_hChangeHandles[0] );
	delete s_pThread;
	s_pThread = NULL;
	TRACE0("CChangeNotification::Cleanup done\n");
}

BOOL CChangeNotification::MonitorStart()
{
	TRACE2("CChangeNotification::MonitorStart %s %s\n", m_strPath, m_strFile );
	m_bRestart = FALSE;
	m_hChangeHandle = FindFirstChangeNotification( m_strPath, m_bWatchSubtree, m_dwNotifyFilter );
	if ( m_hChangeHandle == INVALID_HANDLE_VALUE || m_hChangeHandle == NULL )
	{
		DWORD dwErr = GetLastError();
		TRACE1("CChangeNotification::MonitorStart dwErr=%d\n", dwErr);
		return FALSE;
	}
	return Start( this );
}

BOOL CChangeNotification::MonitorRestart()
{
	TRACE2("CChangeNotification::MonitorRestart <%s> <%s>\n", m_strPath, m_strFile );
	if ( !m_bRestart )
	{
		FindCloseChangeNotification( m_hChangeHandle );
		m_hChangeHandle = INVALID_HANDLE_VALUE;
		m_lChanged = 0;
		m_bRestart = TRUE;
		return FALSE;
	}

	m_hChangeHandle = FindFirstChangeNotification( m_strPath, m_bWatchSubtree, m_dwNotifyFilter );
	if ( m_hChangeHandle == INVALID_HANDLE_VALUE || m_hChangeHandle == NULL )
	{
		DWORD dwErr = GetLastError();
		TRACE1("CChangeNotification::MonitorRestart FindFirstChangeNotification dwErr=%d\n", dwErr);
		return FALSE;
	}
	m_bRestart = FALSE;
	return TRUE;
}

BOOL CChangeNotification::Start( CChangeNotification* pObj )
{
	TRACE0("CChangeNotification::Start\n");
	if ( s_hChangeHandles[0] == NULL )	// interrupt event
	{
		s_hChangeHandles[0] = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( s_hChangeHandles[0] == NULL )
			return FALSE;
	}
	if ( s_pThread == NULL )
	{
		s_pThread = AfxBeginThread( &ThreadProc, NULL, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL );
		s_pThread->m_bAutoDelete = FALSE;
		s_pThread->ResumeThread();
	}
	ASSERT ( s_intr == None );
	SetEvent( s_hChangeHandles[0] );
	CSingleLock singleLock(&s_CS, TRUE);
	AddHandle( pObj );
	return TRUE;
}

BOOL CChangeNotification::Stop( CChangeNotification* pObj )
{
	TRACE0("CChangeNotification::Stop\n");
	if ( s_pThread == NULL )
		return TRUE;		// nothing started

	ASSERT ( s_intr == None );
	SetEvent( s_hChangeHandles[0] );
	CSingleLock singleLock(&s_CS, TRUE);
	RemoveHandle( pObj );
	RemoveHandleRst( pObj );
	return TRUE;
}

BOOL CChangeNotification::CheckWait(DWORD dwWait)
{
	if ( dwWait == WAIT_FAILED ) {
		TRACE1("WAIT_FAILED err=%d\n", GetLastError());
	} else if ( dwWait == WAIT_ABANDONED ) {
		TRACE0("WAIT_ABANDONED\n");
	} else if ( dwWait == WAIT_TIMEOUT ) {
		TRACE0("WAIT_TIMEOUT\n");
	} else {
		TRACE1("dwWait=%d\n",dwWait);
	}
	return FALSE;
}

UINT CChangeNotification::ThreadProc( LPVOID pParam )
{
	// test
	//					DWORD dwWO = WAIT_OBJECT_0;
	//					DWORD dwAO = WAIT_ABANDONED_0;
//	SetEvent( s_hWait );	// allow interrupt
	while( TRUE )
	{
		// TRACE0("CChangeNotification::ThreadProc wait\n");
		CSingleLock singleLock(&s_CS, TRUE);
		// TRACE1("CChangeNotification::ThreadProc ready nHandles=%d\n", s_nHandles);
		DWORD dwWaitStatus = WaitForMultipleObjects( s_nHandles, s_hChangeHandles, FALSE, 10000 );	// 10s instead of INFINITE
		if ( dwWaitStatus == WAIT_FAILED )
		{
			DWORD dwErr = GetLastError();
			TRACE1("CChangeNotification::ThreadProc WAIT_FAILED dwErr=%d\n", dwErr);
//			return 1;
			Sleep( 500 );
			continue;
		}
		if ( dwWaitStatus == WAIT_OBJECT_0 )	// interrupted
		{
			TRACE1("CChangeNotification::ThreadProc intr %d\n", s_intr);
			if ( s_intr == Exit )
				return 0;

			if ( s_intr != None )
				ASSERT( FALSE );
			s_intr = None;
		}
		else if ( dwWaitStatus == WAIT_TIMEOUT )	// timeout: try restart
		{
			TRACE1("CChangeNotification::ThreadProc timeout at %d\n", GetTickCount());
			for ( int n = 0; n < s_nHandlesRst; ++n ) {
				if ( s_pObjRst[n]->m_bRestart ) {
					if ( s_pObjRst[n]->MonitorRestart() ) {
						AddHandle( s_pObjRst[n] );
						Changed( s_pObjRst[n] );
						RemoveHandleRst( s_pObjRst[n] );
					}
				}
			}
		}
		else	// change occurred
		{
			int n = dwWaitStatus - WAIT_OBJECT_0;
			TRACE1("CChangeNotification::ThreadProc change occurred %d\n", n);
			if ( n > 0 && n < s_nHandles )
			{
				if ( s_pObj[n]->m_bEnabled )
				{
					Changed( s_pObj[n] );
				}
				if ( !FindNextChangeNotification( s_hChangeHandles[n] ) )
				{
					DWORD dwErr = GetLastError();
					TRACE1("CChangeNotification::ThreadProc FindNextChangeNotification dwErr=%d\n", dwErr);
					if ( !s_pObj[n]->MonitorRestart() ) {
						s_pObjRst[s_nHandlesRst++] = s_pObj[n];
						ASSERT(s_nHandlesRst <= MAX_CHANGE_HANDLES );
						Changed( s_pObj[n] );
						RemoveHandle( s_pObj[n] );
					}
				}
			}
			else
			{
				ASSERT( FALSE );	// invalid n
			}
		}
		singleLock.Unlock();
		Sleep( 10 );
	}
}

void CChangeNotification::AddHandle( CChangeNotification* pObj )
{
	TRACE1("CChangeNotification::AddHandle %d\n", s_nHandles);
	ASSERT( pObj->m_hChangeHandle != NULL && 
			pObj->m_hChangeHandle != INVALID_HANDLE_VALUE );
	s_pObj[s_nHandles] = pObj;
	s_hChangeHandles[s_nHandles] = pObj->m_hChangeHandle;
	++s_nHandles;
}

void CChangeNotification::RemoveHandle( CChangeNotification* pObj )
{
	for ( int n = 1; n < s_nHandles; ++n )
	{
		if ( pObj == s_pObj[n] )
		{
			TRACE1("CChangeNotification::RemoveHandle %d\n", n);
			for ( int x = n+1; x < s_nHandles; ++x )
			{
				s_pObj[x-1] = s_pObj[x];
				s_hChangeHandles[x-1] = s_hChangeHandles[x];
			}
			--s_nHandles;
			break;
		}
	}
}

void CChangeNotification::RemoveHandleRst( CChangeNotification* pObj )
{
	for ( int n = 0; n < s_nHandlesRst; ++n )
	{
		if ( pObj == s_pObjRst[n] )
		{
			TRACE1("CChangeNotification::RemoveHandleRst %d\n", n);
			for ( int x = n+1; x < s_nHandlesRst; ++x )
			{
				s_pObjRst[x-1] = s_pObjRst[x];
			}
			--s_nHandlesRst;
			break;
		}
	}
}

void CChangeNotification::Changed( CChangeNotification* pObj )
{
	if ( pObj->m_strFile.IsEmpty() )	// dir monitor
	{
		InterlockedExchange( &pObj->m_lChanged, 1 );
	}
	else	// check file status
	{
		struct _stati64 fs;
		int result = _tstati64( pObj->m_strFile, &fs );
		long ch = 0;
		if ( result != 0 || fs.st_mtime != pObj->m_fs.st_mtime )
			ch = 1;
		if ( (fs.st_mode & _S_IWRITE) != (pObj->m_fs.st_mode & _S_IWRITE) )
			ch |= 2;
		if ( ch != 0 ) {
			InterlockedExchange( &pObj->m_lChanged, ch );
			pObj->m_fs = fs;
		}
	}
}
