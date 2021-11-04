#include "StdAfx.h"
#include "DocDirArchive.h"

IMPLEMENT_DYNCREATE(CDocDirArchive, CDocDir)

// CStorage<CDocDirArchive> CDocDirArchive::s_storeDocs( 100, _T("CDocDirArchive") );

CDocDirArchive::CDocDirArchive(void)
{
	TRACE1( "CDocDirArchive::CDocDirArchive() %x\n",  this );
	m_pArcRoot = NULL;
}

//CDocDir* CDocDirArchive::CreateSubDoc( const CString &strName, DOCPOS pos ) 
//{
	// only for empty/dummy dirs
//	ASSERT( pos != NULL );

//	POSITION posDoc = s_storeDocs.New();
//    CDocDirArchive *pDoc = s_storeDocs.GetPtrAt(posDoc);
//	pDoc->m_posStorage = posDoc;
//	TRACE2( "CDocDirArchive::CreateSubDoc( %s ) %x\n", strName, pDoc );
//	pDoc->Init( strName, this, pos );
//	pDoc->m_pArcRoot = m_pArcRoot;
//	return pDoc;

//  return new CDocDirZipSub( strName, this, pos );
//}

CDocDirArchive::~CDocDirArchive(void)
{
//	TRACE2( "CDocDirArchive::~CDocDirArchive() %x %s\n", this, m_strName );
}

//void CDocDirArchive::Delete()
//{
//	if ( m_posStorage != NULL ) {
//		POSITION pos = m_posStorage;
//		m_posStorage = NULL;
//		s_storeDocs.DeleteAt( pos );
//	}
//	else
//		delete this;
//}

void CDocDirArchive::DeleteContents()
{
	ResetAll();
	CDocDir::DeleteContents();
}

void CDocDirArchive::SetModifiedFlag( DOCPOS pos, BOOL bModified /* = TRUE */ )
{
	TRACE2("CDocDirArchive::SetModifiedFlag(%d) <%s>\n", bModified, m_strName );
	CDocDir::SetModifiedFlag( pos, bModified );
	if ( bModified )
	{
		GetDirEntry(pos).SetNoExtr( FALSE );
		CDocDir::SetModifiedFlag();
	}
}

BOOL CDocDirArchive::ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE1( "CDocDirArchive::ReScanAuto merge %s\n", GetPathName() );
	BOOL bNew = FALSE;
	CDocDir::CMerge merge( m_list, m_listArcNew );
	while ( merge.HasMore() )
	{
		int nComp = merge.CompareDoc();
		TRACE3( " %d %s/%s\n", nComp, merge.GetNameL(), merge.GetNameR() );
		if ( nComp < 0 )
		{
			CDirEntry &de = merge.GetDirEntryL();
			if ( !de.IsDel() )
			{
				de.SetDel();
				bNew = TRUE;
			}
		}
		else if ( nComp == 0)
		{
			CDirEntry &de = merge.GetDirEntryL();
			CDirEntry &deNew = merge.GetDirEntryR();
			if ( deNew.IsDel() )		// 2011/03/23
			{
				de.SetDel();
				bNew = TRUE;
			} 
			else if ( de.CopyAttr( deNew ) )
			{
				de.SetModif( deNew.IsModif() );
				de.SetNoExtr( deNew.IsNoExtr() );
				if ( de.GetName() == _T("..") )
					de.SetMark( FALSE );
				else if ( !de.IsDir() )
					bNew = TRUE;
			}
			CDocDir *pd = de.GetDoc();	// check dependent folders
			if ( pd != NULL ) {
				CDocDir *pdNew = deNew.GetDoc();
				if (pdNew != NULL) {
					ASSERT_KINDOF( CDocDirArchive, pd );
					CDocDirArchive *pdz = (CDocDirArchive*)pd;
					if ( pdz->m_listArcNew.IsEmpty() ) {
						ASSERT_KINDOF( CDocDirArchive, pdNew );
						CDocDirArchive *pdzn = (CDocDirArchive*)pdNew;
						pdz->m_listArcNew.Copy( &pdzn->m_listArcNew );
					}
				}
				bNew = TRUE;
				de.SetCopied();	// 20120620
				de.SetMark();
			}
			TRACE2( "   %4.4x %4.4x\n", de.GetFlags(), deNew.GetFlags() );
		}
		else
		{
			merge.CopyR2L();
			bNew = TRUE;
			nComp = 0;
		}
		merge.GetNext( nComp );
	}
	return bNew;
}


CString CDocDirArchive::GetFullPath( const CString &strDir ) 
{
	CString strPath = GetPathName();
	if ( strPath.Right( 1 ) != "\\" )
		strPath += "\\";
	strPath += strDir;
	return strPath;
}

CString CDocDirArchive::GetFullPathEx( const CString &strName, BOOL bExtract,
	LPPROGRESS_ROUTINE lpProgressRoutine /* = NULL */, LPVOID lpData /* = NULL */ )
{
	ASSERT( m_pArcRoot != NULL );
	// assuming strName is marked to be extracted
	CString	strPathEx = m_pArcRoot->GetTempPath( GetSubPath() + '/' + strName, bExtract, lpProgressRoutine, lpData );
	strPathEx.Replace( _T("/"), _T("\\") );
	return strPathEx;
}

CString CDocDirArchive::GetSubPath() 
{
	ASSERT_KINDOF( CDocDirArchive, GetParentDoc() );
	ASSERT( m_pArcRoot != NULL );
	if ( GetParentDoc() == m_pArcRoot )
		return m_strName;
	else
		return ((CDocDirArchive*)GetParentDoc())->GetSubPath() + '\\' + m_strName;
}

void CDocDirArchive::GetMarkedList( CStringArray &astrFiles, __int64 &nSize, BOOL bInclDirs /* =FALSE */ )
{
	CString strSubPath;
	if ( m_pArcRoot == this )
		strSubPath = m_pArcRoot->GetFilter();
	else
		strSubPath = GetSubPath() + '\\';

	DOCPOS pos = m_list.GetHeadPosition();
	while ( pos != NULL )
	{
		CDirEntry &de = m_list.GetAt( pos );
#ifdef _DEBUG
		if ( de.IsMarked() ) {
			TRACE1( "M> %s\n", strSubPath + de.GetName() );
		}
#endif
		if ( de.IsDir() )
		{
			if ( bInclDirs && de.IsMarked() )
				astrFiles.Add( strSubPath + de.GetName() + '\\' );
			CDocDirArchive *pd = (CDocDirArchive *)de.GetDoc();
			if ( pd != NULL )
			{
				ASSERT_KINDOF( CDocDirArchive, pd );
				pd->GetMarkedList( astrFiles, nSize, bInclDirs );
			}
		}
		else if ( de.IsMarked() )
		{
			astrFiles.Add( strSubPath + de.GetName() );
			nSize += de.GetFileSize();
		}
		m_list.GetNext( pos );
	}
}

BOOL CDocDirArchive::RemoveFileDir( DOCPOS pos )
{
	if ( pos == NULL )
		return FALSE;

	CDirEntry &de = m_list.GetAt(pos);
	DOCPOS posNew = m_listArcNew.Find( de.IsDir(), de.GetName() );
	if ( posNew != NULL ) {
		CDirEntry &deN = m_listArcNew.GetAt(posNew);
		deN.SetDel();
	}
//	SetModifiedFlag( pos );
	CDocDir::SetModifiedFlag();
	CDocDir *pd = de.GetDoc();	// 2011/03/17
	if ( pd != NULL )
		pd->SetModifiedFlag( FALSE ); // prevent saving
	if ( de.IsNoExtr() )
		return TRUE;
	BOOL b = CDocDir::RemoveFileDir( pos );
	if ( b )
		de.SetDel( FALSE );		// undo required for later ReScanAuto
	return b;
}

BOOL CDocDirArchive::CopyFile( DOCPOS posDest, CDocDir *pDocDirSource, DOCPOS posSource, BOOL bPhysCopy,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	BOOL b = CDocDir::CopyFile( posDest, pDocDirSource, posSource, bPhysCopy, lpProgressRoutine, lpData );
	if ( bPhysCopy && b ) {
		CDirEntry &deSrc = pDocDirSource->GetList().GetAt(posSource);
		DOCPOS posDestN = m_listArcNew.Find( FALSE, deSrc.GetName() );
		if ( posDestN == NULL ) {
			posDestN = m_listArcNew.Insert( CDirEntry( deSrc.IsDir(), deSrc.GetName(), FALSE, deSrc.GetDateTime(), deSrc.GetFileSize() ) );
			AssureDirArcNew();
		} else {
			m_listArcNew.GetAt( posDestN ).Copy( deSrc ); 
			m_listArcNew.GetAt( posDestN ).SetRO( FALSE );
		}
		m_listArcNew.GetAt( posDestN ).SetModif();
		if ( posDest != NULL )
			SetModifiedFlag( posDest );

		CDocDir::SetModifiedFlag();
	}
	return b;
}

void CDocDirArchive::AssureDirArcNew()
{
	if ( GetParentDoc() == NULL || !GetParentDoc()->IsKindOf(RUNTIME_CLASS( CDocDirArchive )) )
		return;
	TRACE1("CDocDirArchive::AssureDirArcNew %s\n", m_strName );
	CDocDirArchive* pParent = (CDocDirArchive*)GetParentDoc();
	DOCPOS pos = pParent->m_listArcNew.Find( TRUE, m_strName );
	if ( pos != NULL ) {
		pParent->m_listArcNew.GetAt(pos).SetDel(FALSE);
	} else {
		pParent->m_listArcNew.Insert( CDirEntry( (m_pArcRoot != this), m_strName, TRUE ) );
	}
	pParent->AssureDirArcNew();
}



