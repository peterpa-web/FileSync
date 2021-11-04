#pragma once
#include "Storage.h"
#include "DualTreeDir.h"

class CTaskIdle : protected CStorageObj
{
public:
	CTaskIdle();
	~CTaskIdle();
	virtual void Delete() { ASSERT(FALSE); }
	virtual int ProcessStep(){ ASSERT(FALSE); return 1; }
	virtual int Compare( CTaskIdle *pTask ) { return 1; } // default: no match

protected:
	CDualTreeDir *m_pTree;
	TREEPOS m_posParent;
	TREEPOS m_pos;	// current step
//	BOOL m_bCanceled;
	void *m_pStore;

	void Init(CDualTreeDir *pTree, TREEPOS posParent);
	void operator delete( void *p ) { ASSERT( FALSE ); }  // disable public usage  	
};

