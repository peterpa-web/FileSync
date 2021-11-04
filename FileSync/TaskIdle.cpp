#include "StdAfx.h"
#include "TaskIdle.h"


CTaskIdle::CTaskIdle()
{
	m_pos = NULL;
//	m_bCanceled = FALSE;
	m_pStore = NULL;
}

CTaskIdle::~CTaskIdle()
{
	ASSERT( m_posStorage == NULL );  // should be called from Delete() only
}

void CTaskIdle::Init(CDualTreeDir *pTree, TREEPOS posParent) 
{
	m_pTree = pTree;
	m_posParent = posParent;
}

