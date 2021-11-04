#include "StdAfx.h"
#include "UndoTask.h"

#include "UndoBuffer.h"

CUndoBuffer::CUndoBuffer(void)
{
	m_nTask = 0;
	m_nMaxSize = -1;	// unlimited
	m_nCurrSize = 0;
}

CUndoBuffer::~CUndoBuffer(void)
{
	RemoveAll();
}

BOOL CUndoBuffer::CanUndo(void)
{
	return m_nTask > 0;
}

BOOL CUndoBuffer::CanRedo(void)
{
	return m_apTask.GetSize() > m_nTask;
}

BOOL CUndoBuffer::AddTask(CUndoTask* pTask)
{
	if ( !pTask->DoFirst() )
		return FALSE;
	if ( !pTask->Do() )
		return FALSE;

	RemoveAt( m_nTask );	// kill redo items if present

	// check size ... remove at 0
	INT_PTR nSize = pTask->GetSize();
	while ( m_nMaxSize > 0 && m_nTask > 0 && ( m_nCurrSize + nSize ) > m_nMaxSize )
	{
		TRACE( "CUndoBuffer::AddTask removing old task %d\n", m_nTask );
		RemoveAt(0,1);
		--m_nTask;
	}
	m_apTask.Add( pTask );
	++m_nTask;
	m_nCurrSize += nSize;
	return TRUE;
}

BOOL CUndoBuffer::Undo(void)
{
	if ( m_nTask <= 0 )
		return FALSE;
	if ( m_apTask[m_nTask - 1]->Undo() )
	{
		--m_nTask;
		return TRUE;
	}
	return FALSE;
}

BOOL CUndoBuffer::Redo(void)
{
	if ( m_nTask >= m_apTask.GetSize() )
		return FALSE;
	if ( m_apTask[m_nTask]->Do() )
	{
		++m_nTask;
		return TRUE;
	}
	return FALSE;
}

void CUndoBuffer::RemoveAll(void)
{
	for ( int n = 0; n < m_apTask.GetSize(); ++n )
		delete m_apTask[ n ];
	m_apTask.RemoveAll();
	m_nTask = 0;
//	m_nMaxSize = -1;	// unlimited
	m_nCurrSize = 0;
}

void CUndoBuffer::RemoveAt(int nStart, int nCount /* =-1 */ )
{
	if ( nCount < 0 )
		nCount = (int)m_apTask.GetSize();
	else
		nCount += nStart;
	for ( int n = nStart; n < nCount; ++n )
	{
		m_nCurrSize -= m_apTask[n]->GetSize();
		delete m_apTask[ n ];
	}
	m_apTask.RemoveAt( nStart, m_apTask.GetSize()-nStart );
}

void CUndoBuffer::CleanModifiedFlag(int nChan)
{
	for ( int n = 0; n < m_apTask.GetSize(); ++n )
	{
		m_apTask[ n ]->SetModifiedFlag( nChan, (n != m_nTask) );
	}
}

