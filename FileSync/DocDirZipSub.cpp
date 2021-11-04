#include "StdAfx.h"
//#include "DocDirZipSub.h"
#include "DocDirZipRoot.h"

IMPLEMENT_DYNCREATE(CDocDirZipSub, CDocDirArchive)

CStorage<CDocDirZipSub> CDocDirZipSub::s_storeDocs( 100, _T("CDocDirZipSub") );

CDocDirZipSub::CDocDirZipSub(void)
{
	TRACE1( "CDocDirZipSub::CDocDirZipSub() %x\n",  this );
}

CDocDir* CDocDirZipSub::CreateSubDoc( const CString &strName, DOCPOS pos ) 
{
	// only for empty/dummy dirs
	ASSERT( pos != NULL );

	POSITION posDoc = s_storeDocs.New();
    CDocDirZipSub *pDoc = s_storeDocs.GetPtrAt(posDoc);
	pDoc->m_posStorage = posDoc;
	TRACE2( "CDocDirZipSub::CreateSubDoc( %s ) %x\n", strName, pDoc );
	pDoc->Init( strName, this, pos );
	pDoc->m_pArcRoot = m_pArcRoot;
	return pDoc;

//  return new CDocDirZipSub( strName, this, pos );
}

CDocDirZipSub::~CDocDirZipSub(void)
{
	TRACE2( "CDocDirZipSub::~CDocDirZipSub() %x %s\n", this, m_strName );
}

void CDocDirZipSub::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeDocs.DeleteAt( pos );
	}
	else
		delete this;
}

void CDocDirZipSub::DeleteContents()
{
	ResetAll();
	CDocDir::DeleteContents();
}

void CDocDirZipSub::SetModifiedFlag( DOCPOS pos, BOOL bModified /* = TRUE */ )
{
	TRACE2("CDocDirZipSub::SetModifiedFlag(%d) <%s>\n", bModified, m_strName );
    CDocDirArchive::SetModifiedFlag( pos, bModified );
}

void CDocDirZipSub::InsertPath( CListDirEntries &list, CString strSubPath, const CZipFileEntry fe )
{
	if ( strSubPath.IsEmpty() )
		return;

	// int p = strSubPath.Find( '/' );
	int p = strSubPath.Find( '\\' );
	if ( p < 0 ) {
		list.Insert( CDirEntry( FALSE, strSubPath, TRUE, fe.dt, fe.dwSize, fe.crc, TRUE ) );
		return;
	}

	CString strDir;
	strDir = strSubPath.Left( p );
	strSubPath = strSubPath.Mid( p + 1 );
	DOCPOS pos = list.Find( TRUE, strDir );		// find dir
	if ( pos == NULL )
		pos = list.Insert( CDirEntry( TRUE, strDir, TRUE ) );

	CDirEntry &de = list.GetAt( pos );
	if ( strSubPath.IsEmpty() )
	{
		de.SetDateTime( fe.dt );					// update attributes
	}
	else
	{												// process subdir
		CDocDir *pd = de.GetDoc();
		if ( pd == NULL )
		{
			pd = CreateSubDoc( strDir, pos );
			de.SetDoc( pd );
		}
		ASSERT_KINDOF( CDocDirZipSub, pd );
		CDocDirZipSub *pdz = (CDocDirZipSub*)pd;
		if ( &m_list == &list )
			pdz->InsertPath( pdz->m_list, strSubPath, fe );
		else
			pdz->InsertPath( pdz->m_listArcNew, strSubPath, fe );
	}
}


