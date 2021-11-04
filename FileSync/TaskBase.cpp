#include "StdAfx.h"
#include "TaskBase.h"


CTaskBase::CTaskBase()
{
//	m_bCanceled = FALSE;
	m_nSide = 2;	// both
	m_nProgrBytesAll = 10000;
}


CTaskBase::~CTaskBase()
{
}

void CTaskBase::Init(BOOL bMain)
{
	m_bMain = bMain;
}
