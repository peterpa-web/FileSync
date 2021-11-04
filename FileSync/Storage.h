#pragma once
#include "afxtempl.h"
#include "StackWalker.h"
#include "afxmt.h"

#ifdef _DEBUG

class StackWalkerToTrace : public StackWalker
{
protected:
  virtual void OnOutput(LPCSTR szText);
};

void AssertX(BOOL b);
void AssertX1(BOOL b, LPCSTR szText);

#undef ASSERT
#define ASSERT(f) ::AssertX(f)
#define ASSERT1(f,t) ::AssertX1(f,t)
#else
#define AssertX(f)      ((void)0)
#define AssertX1(f,t)   ((void)0)
#define ASSERT1(f,t)   ((void)0)
#endif

class CStorageObj 
{
public:
	CStorageObj() { m_posStorage = NULL; }
	~CStorageObj() { AssertX( m_posStorage == NULL ); }  // should be called from Delete() only

protected:
	POSITION m_posStorage;
};

// ################

template<class TYPE, class ARG_TYPE = const TYPE&>
class CStorage :
	protected CList<TYPE,ARG_TYPE>
{
public:
	CStorage( INT_PTR nBlockSize, LPCTSTR pszType ) : CList( nBlockSize ) { m_pszType = pszType; m_nMaxBlocks = 0; }
	~CStorage() { TRACE2( "~CStorage %s n=%d\n", m_pszType, m_nMaxBlocks ); }
	CCriticalSection m_CSBusy;
	POSITION New();
	TYPE* GetPtrAt(POSITION pos);
	void DeleteAt(POSITION pos) {
		CSingleLock singleLockTaskList(&m_CSBusy, TRUE);
		RemoveAt(pos); }
	INT_PTR GetCount() { return CList<TYPE,ARG_TYPE>::GetCount(); };
	TYPE& GetHead() { return CList<TYPE,ARG_TYPE>::GetHead(); }
#ifdef _DEBUG
	void AssertValid() const;
	int GetBlocks() const;
#endif

private:
	LPCTSTR m_pszType;
	int m_nMaxBlocks;
};

template<class TYPE, class ARG_TYPE>
POSITION CStorage<TYPE, ARG_TYPE>::New()
{
	CSingleLock singleLockTaskList(&m_CSBusy, TRUE);
	ASSERT_VALID(this);
	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
//	pNewNode->data.m_posStorage = (POSITION) pNewNode;
#ifdef _DEBUG
	int nBlocks = GetBlocks();
	if ( nBlocks > m_nMaxBlocks ) m_nMaxBlocks = nBlocks;
#endif
	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE TYPE* CStorage<TYPE, ARG_TYPE>::GetPtrAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
		return &(pNode->data); }
		
#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CStorage<TYPE, ARG_TYPE>::AssertValid() const
{
	CObject::AssertValid();

	if (m_nCount == 0)
	{
		// empty list - problem with RemoveAt -> RemoveAll where m_pNodeHead is not always clean
		// AssertX(m_pNodeHead == NULL);
		// AssertX(m_pNodeTail == NULL);
	}
	else
	{
		// non-empty list
		AssertX(AfxIsValidAddress(m_pNodeHead, sizeof(CNode)));
		AssertX(AfxIsValidAddress(m_pNodeTail, sizeof(CNode)));
	}
}

template<class TYPE, class ARG_TYPE>
int CStorage<TYPE, ARG_TYPE>::GetBlocks() const
{
	int n = 0;
	struct CPlex* pBlock = m_pBlocks;
	while ( pBlock != NULL ) {
		++n;
		pBlock = pBlock->pNext;
	}
	return n;
}
#endif //_DEBUG

