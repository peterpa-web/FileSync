#include "StdAfx.h"
//#include "DocDirIsoSub.h"
#include "DocDirIsoRoot.h"

IMPLEMENT_DYNCREATE(CDocDirIsoSub, CDocDirArchive)

CStorage<CDocDirIsoSub> CDocDirIsoSub::s_storeDocs( 100, _T("CDocDirIsoSub") );

CDocDirIsoSub::CDocDirIsoSub(void)
{
	TRACE1( "CDocDirIsoSub::CDocDirIsoSub() %x\n",  this );
}

CDocDir* CDocDirIsoSub::CreateSubDoc( const CString &strName, DOCPOS pos ) 
{
	// only for empty/dummy dirs
	ASSERT( pos != NULL );

	POSITION posDoc = s_storeDocs.New();
    CDocDirIsoSub *pDoc = s_storeDocs.GetPtrAt(posDoc);
	pDoc->m_posStorage = posDoc;
	TRACE2( "CDocDirIsoSub::CreateSubDoc( %s ) %x\n", strName, pDoc );
	pDoc->Init( strName, this, pos );
	pDoc->m_pArcRoot = m_pArcRoot;
	return pDoc;

//  return new CDocDirIsoSub( strName, this, pos );
}

CDocDirIsoSub::~CDocDirIsoSub(void)
{
	TRACE2( "CDocDirIsoSub::~CDocDirIsoSub() %x %s\n", this, m_strName );
}

void CDocDirIsoSub::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeDocs.DeleteAt( pos );
	}
	else
		delete this;
}

void CDocDirIsoSub::DeleteContents()
{
	ResetAll();
	CDocDir::DeleteContents();
}

void CDocDirIsoSub::SetModifiedFlag( DOCPOS pos, BOOL bModified /* = TRUE */ )
{
	TRACE2("CDocDirIsoSub::SetModifiedFlag(%d) <%s>\n", bModified, m_strName );
    CDocDirArchive::SetModifiedFlag( pos, bModified );
}

void CDocDirIsoSub::InsertPath( CListDirEntries &list, IsoDirectory &isoDir )
{
	int nMax = isoDir.files.GetCount();
	for ( int n = 0; n < nMax; ++n )
	{
		const IsoFileDescriptor &fd = isoDir.files.GetAt(n);
		TRACE1( "  %s\n", fd.name );
		CTime dt = CTime(fd.date.year,fd.date.month,fd.date.day,fd.date.hour,fd.date.minute,fd.date.second,fd.date.gmtOffset);

		if ( !fd.IsDir() ) {
			CString strName = fd.name;
			int p = strName.Find( ';' );
			if ( p > 0 )
				strName = strName.Left( p );
			list.Insert( CDirEntry( FALSE, strName, TRUE, dt, fd.size, 0, TRUE ) );
			continue;
		}

		CString strDir = fd.name;
		if ( strDir == _T(".") || strDir == _T("..") )
			continue;

		DOCPOS pos = list.Insert( CDirEntry( TRUE, strDir, TRUE, dt ) );

		CDirEntry &de = list.GetAt( pos );
		CDocDir *pd = de.GetDoc();
		if ( pd == NULL )
		{
			pd = CreateSubDoc( strDir, pos );
			de.SetDoc( pd );
		}
		ASSERT_KINDOF( CDocDirIsoSub, pd );
		CDocDirIsoSub *pdz = (CDocDirIsoSub*)pd;
		IsoDirectory isoSubDir( isoDir.GetReader(), fd );
		if ( &m_list == &list )
			pdz->InsertPath( pdz->m_list, isoSubDir );
		else
			pdz->InsertPath( pdz->m_listArcNew, isoSubDir );
	}	// next n
}


