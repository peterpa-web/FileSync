#pragma once
#include "Storage.h"

class CTaskBase : protected CStorageObj
{
	friend class CThreadBack;

public:
	CTaskBase();
	~CTaskBase();
	virtual void Delete() { ASSERT(FALSE); }
	virtual UINT Process(CThreadBack *pThreadBack) { ASSERT(FALSE); return 1; }
	virtual int Compare( CTaskBase *pTask ) { return 1; } // default: no match
	virtual __int64 NewProgrBytesAll() { return m_nProgrBytesAll; }

	int GetSide() const { return m_nSide; }
	BOOL IsMainTask() const { return m_bMain; }
	__int64 GetProgrBytesAll() const { return m_nProgrBytesAll; }

protected:
//	BOOL m_bCanceled;
	int m_nSide;
	BOOL m_bMain;
	void *m_pStore;
	__int64 m_nProgrBytesAll;

	void Init(BOOL bMain);
	void operator delete( void *p ) { ASSERT( FALSE ); }  // disable public usage  	
};

