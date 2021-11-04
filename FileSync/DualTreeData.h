#pragma once
#include "afxmt.h"
#include "afxtempl.h"

typedef POSITION TREEPOS;

template<class ITYPE>		// ITYPE must be type of CDualTreeItem
class CDualTreeData :
	protected CList<ITYPE,const ITYPE&>
{
protected:
	CDualTreeData(void);
	~CDualTreeData(void);
	TREEPOS CreateItem( const ITYPE &d, TREEPOS posAfter );
	BOOL DeleteItem( TREEPOS pos );
	BOOL DeleteAllItems();
//	TREEPOS GetFirstPos() const { return GetHeadPosition(); }
//	TREEPOS GetNextPos( TREEPOS pos ) const { GetNext( pos ); return pos; }

public:
	ITYPE& GetItemData( TREEPOS pos );
	const ITYPE& GetItemData( TREEPOS pos ) const;
#ifdef _DEBUG
	void AssertValid() const;
#endif

protected:
	CCriticalSection m_critSection; // protect list ptr changes 
};

template<class ITYPE>
CDualTreeData<ITYPE>::CDualTreeData(void)
{
}

template<class ITYPE>
CDualTreeData<ITYPE>::~CDualTreeData(void)
{
}

template<class ITYPE>
TREEPOS CDualTreeData<ITYPE>::CreateItem( const ITYPE &d, TREEPOS posAfter )
{
	CSingleLock singleLock(&m_critSection, TRUE);
	POSITION pos;
	if ( posAfter == NULL )
		pos = m_lItemData.AddHead( d );
	else
		pos = m_lItemData.InsertAfter( posAfter, d );
	return pos;
}

template<class ITYPE>
BOOL CDualTreeData<ITYPE>::DeleteItem( TREEPOS pos )
{
	HTREEITEM hItem = GetAt( pos ).GetItemHandle();
	if ( hItem != NULL ) {
//		GetAt( pos ).SetMarkDel();   ??? GetAt( pos ).ResetDocRef(0+1);
		return FALSE;
	}
	VERIFY( GetAt( pos ).IsDeleted() );
	RemoveAt( pos );
	return TRUE;
}

template<class ITYPE>
BOOL CDualTreeData<ITYPE>::DeleteAllItems( )
{
	RemoveAll();
	return TRUE;
}

template<class ITYPE>
ITYPE& CDualTreeData<ITYPE>::GetItemData( TREEPOS pos )
{
	return GetAt( pos );
}

template<class ITYPE>
const ITYPE& CDualTreeData<ITYPE>::GetItemData( TREEPOS pos ) const
{
	return GetAt( pos );
}

#ifdef _DEBUG
template<class ITYPE>
void CDualTreeData<ITYPE>::AssertValid() const
{
	CObject::AssertValid();

	if (m_nCount == 0)
	{
		// empty list
		ASSERT(m_pNodeHead == NULL);
		ASSERT(m_pNodeTail == NULL);
	}
	else
	{
		// non-empty list
		ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(CNode)));
		ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(CNode)));
	}
}
#endif //_DEBUG

