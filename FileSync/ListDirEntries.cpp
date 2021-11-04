#include "StdAfx.h"
#include "DocDir.h"

#include "ListDirEntries.h"


void CListDirEntries::RemoveAt( POSITION pos )
{
	if ( pos == m_pos ) 
		m_pos = NULL;
	CListDirEntriesBase::RemoveAt( pos );
}

void CListDirEntries::RemoveAll()
{	
	m_pos = NULL;
	//CListDirEntriesBase::RemoveAll();
	ASSERT_VALID(this);

	// destroy elements backwards
	CNode* pNode;
	for (pNode = m_pNodeTail; pNode != NULL; pNode = pNode->pPrev) {
//		pNode->data.SetViewItemPos(NULL);
		ASSERT(pNode->data.GetViewItemPos() == NULL );
		pNode->data.~CDirEntry();
	}

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

void CListDirEntries::ResetAll()
{
	POSITION pos = GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &cd = GetAt( pos );
		cd.SetViewItemPos( NULL );		// 20120312
		CDocDir *pd = cd.GetDoc();
		if ( pd != NULL ) {
			pd->ResetAll();
			GetNext( pos );
		} else {
			POSITION posOld = pos;
			GetNext( pos );
			RemoveAt( posOld );
		}
	}
}

POSITION CListDirEntries::Insert( const CDirEntry &de )
{
//    TRACE1( "%s\n", de.strName );
    if ( m_pos == NULL )
		m_pos = GetHeadPosition();

    if ( m_pos == NULL )
    {
        m_pos = AddTail( de );
    }
    else
    {
		int c = 0;
        if ( GetAt( m_pos ).Compare( de ) < 1 )
        {
            while ( m_pos != NULL )
			{
				c = GetAt( m_pos ).Compare( de );
				if ( c < 1 )
					GetNext( m_pos );
				else
					break;
			}
            if ( m_pos != NULL )
			{
				if ( c != 0 )
	                m_pos = InsertBefore( m_pos, de );
				else
					ASSERT( FALSE );
            }
			else
                m_pos = AddTail( de );
        }
        else
        {
            while ( m_pos != NULL )
			{
				c = GetAt( m_pos ).Compare( de );
				if ( c > -1 )
					GetPrev( m_pos );
				else
					break;
			}
            if ( m_pos != NULL )
			{
 				if ( c != 0 )
					m_pos = InsertAfter( m_pos, de );
				else
					ASSERT( FALSE );
			}
            else
                m_pos = AddHead( de );
        }
    }
	return m_pos;
}

POSITION CListDirEntries::Find( BOOL bDir, const CString &strName ) const
{
	CDirEntry de;
	de.SetName( strName );
	de.SetDir( bDir );
	return Find( de );
}

POSITION CListDirEntries::Find( const CDirEntry &de ) const
{
	POSITION pos = GetHeadPosition();
	while ( pos != NULL )
	{
		const CDirEntry &cd = GetAt( pos );
		if ( cd.Compare( de ) == 0 )
			return pos;

		if ( de.IsDir() && !cd.IsDir() )
			return NULL;	// dir not found

		GetNext( pos );
	}
	return NULL;
}

POSITION CListDirEntries::Find( const CDocDir *pDir ) const
{
	POSITION pos = GetHeadPosition();
	while ( pos != NULL )
	{
		const CDirEntry &cd = GetAt( pos );
		if ( cd.IsDir() || cd.IsZip() )
		{
			const CDocDir *pd = cd.GetDoc();
			if ( pd == pDir )
				return pos;
			if ( pd != NULL )
				return pd->FindDir(pDir);
		}
		GetNext( pos );
	}
	return NULL;
}

void CListDirEntries::SetFlags( DWORD maskCond, DWORD maskRes, DWORD maskSet )
{
	POSITION pos = GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = GetAt( pos );
		if ( de.IsAny(maskCond) )
		{
			de.ResetFlags( maskRes );
			de.SetFlags( maskSet );
		}
		if ( de.IsDir() )
		{
			CDocDir *pd = de.GetDoc();
			if ( pd != NULL )
				pd->GetList().SetFlags( maskCond, maskRes, maskSet );
		}
		GetNext( pos );
	}
}

void CListDirEntries::Copy(CListDirEntries* pSrc)
{
	RemoveAll();
	POSITION posSrc = pSrc->GetHeadPosition();
	while ( posSrc != NULL )
	{
		CDirEntry &deSrc = pSrc->GetAt( posSrc );
		CDocDir *pd = deSrc.GetDoc();	// save doc during copy
		deSrc.SetDoc( NULL );
		POSITION pos = AddTail(deSrc);
		if ( pd != NULL ) {
			CDirEntry &de = GetAt(pos);
			de.SetDoc(pd);
		}
		pSrc->GetNext( posSrc );
	}
}

#ifdef _DEBUG
void CListDirEntries::AssertValid() const
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
