#include "StdAfx.h"

#include "DocDirNative.h"

IMPLEMENT_DYNCREATE(CDocDirNative, CDocDir)

CStorage<CDocDirNative> CDocDirNative::s_storeDocs( 100, _T("CDocDirNative") );
BOOL CDocDirNative::s_bHidePJ = FALSE;

CDocDirNative::CDocDirNative(void)
{
//	TRACE0( "CDocDirNative::CDocDirNative()\n" );
	m_dwLastScan = 0;
//	m_nMaxLineLen = 0;
}

CDocDir* CDocDirNative::CreateSubDoc( const CString &strName, DOCPOS pos ) 
{
	ASSERT( pos != NULL );

	POSITION posDoc = s_storeDocs.New();
    CDocDirNative *pDoc = s_storeDocs.GetPtrAt(posDoc);
	pDoc->m_posStorage = posDoc;
	TRACE2( "CDocDirNative::CreateSubDoc( %s ) %x\n", strName, pDoc );
	pDoc->Init( strName, this, pos );
	return pDoc;
}

CDocDirNative::~CDocDirNative(void)
{	
	TRACE2( "CDocDirNative::~CDocDirNative() %x %s\n", this, m_strName );
	CDocFileSync::SetModifiedFlag( FALSE );
}

void CDocDirNative::Delete()
{
	if ( m_posStorage != NULL ) {
		POSITION pos = m_posStorage;
		m_posStorage = NULL;
		s_storeDocs.DeleteAt( pos );
	}
	else
		delete this;
}

void CDocDirNative::DeleteContents()
{
	CDocDir::DeleteContents();
}

// CDocFileSync serialization

void CDocDirNative::Serialize(CArchive& ar)
{
    ASSERT( FALSE );
}

BOOL CDocDirNative::PreReScanAuto( int nSide, const CString &strBasePath )
{
	return PreReScanAutoBase( nSide, strBasePath, _S_IFDIR );
}

/*
void CDocDirNative::ReScanPath( int nSide, BOOL bRoot, BOOL &bCanceled )
{
	if ( !SaveModified( nSide ) )
		return;

	TRACE0( "ReScanPath\n" );
	m_list.RemoveAll();		// 070619
//	m_nMaxLineLen = 0;
	CString strBasePath = GetPathName();
	if ( !strBasePath.IsEmpty() )
	{
		ScanPathInt( m_list, strBasePath, bCanceled, bRoot );		// 070619
	}
}
*/

BOOL CDocDirNative::ReScanAuto( BOOL bRoot, const int &nCancel, const int nCancelMask,
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
//	ASSERT( !IsModified() );
//	if ( !SaveModified() )			TODO
//		return FALSE;

	if ( m_bScanned ) {
		TRACE0( " scanned\n" );
		return FALSE;	// no change
	}

	SET_DOC_LOCK(TRUE);
	m_listNew.RemoveAll();
	CString strBasePath = GetPathName();
	if ( strBasePath.IsEmpty() && GetParentDoc() != NULL )
	{
		strBasePath = GetParentDoc()->GetFullPath( m_strName );
	}
	if ( !strBasePath.IsEmpty() )
	{
		ScanPathInt( m_listNew, strBasePath, nCancel, nCancelMask, bRoot );
	}
	SET_DOC_LOCK(FALSE);
	if ( (nCancel & nCancelMask) != 0 )
		return FALSE;

//	TRACE1( "CDocDirNative::ReScanAuto %s\n", strBasePath );
	BOOL bNew = FALSE;
	CDocDir::CMerge merge( m_list, m_listNew );
	while ( merge.HasMore() )
	{
		int nComp = merge.CompareDoc();
//		TRACE3( " %d %s/%s\n", nComp, merge.GetNameL(), merge.GetNameR()  );
		if ( nComp < 0 )
		{
			CDirEntry &de = merge.GetDirEntryL();
			if ( !de.IsDel() )
			{
				TRACE1( " setDel %s\n", merge.GetNameL()  );
				de.SetDel();
				bNew = TRUE;
			}
		}
		else if ( nComp == 0)
		{
			CDirEntry &de = merge.GetDirEntryL();
			CDirEntry &deNew = merge.GetDirEntryR();
			if ( de.CopyAttr( deNew ) )
			{
				if ( de.GetName() == _T("..") )
					de.SetMark( FALSE );
				else if ( !de.IsDir() ) {
					bNew = TRUE;
				}
			}
			if ( de.GetDoc() != NULL ) {
				bNew = TRUE;	// check dependent folders
				TRACE1( " chk folder %s\n", merge.GetNameL()  );
				de.SetCopied();		// 2012-01-26
				de.SetMark();
			}
		}
		else
		{
			merge.CopyR2L();
			bNew = TRUE;
			nComp = 0;
//			TRACE1( " add %s\n", merge.GetNameR()  );
		}
		merge.GetNext( nComp );
	}
	m_listNew.RemoveAll();
	m_bScanned = TRUE;
	m_dwLastScan = GetTickCount();
	return bNew;
}

void CDocDirNative::ScanPathInt( CListDirEntries &list, const CString &strPath, const int &nCancel, int nCancelMask, BOOL bRoot )
{
	CFileFind findbase;
	BOOL bFindBase = findbase.FindFile(strPath);
	if (bFindBase) {
		bFindBase = findbase.FindNextFile();
		if ( findbase.GetFilePath() != strPath )
			return;		// case mismatch
	}

	TRACE1( "CDocDirNative::ScanPathInt %s\n", strPath );
	CFileFind finder;
	BOOL bWorking = finder.FindFile( strPath + _T("/*.*") );
	while ( bWorking && (nCancel & nCancelMask) == 0 )
	{
		bWorking = finder.FindNextFile();

		if ( !bRoot && finder.IsDots() )		// allow only for root
			continue;

		if ( s_bHidePJ && !finder.IsDirectory() && finder.GetFileName().Right(3) == _T(".pj") )
			continue;	// hide MKS project files

		CDirEntry de;
		de.SetName( finder.GetFileName() );
		if ( finder.GetFileName() == _T(".") )
			continue;				// ignore

//		TRACE1( " %s\n", de.GetName() );

		FILETIME ft;
		if ( finder.GetLastWriteTime( &ft ) )
		{
			de.SetDateTime( CTime( ft ) );
		}
		de.SetFileSize( finder.GetLength() );
		de.SetDir( finder.IsDirectory() );
		de.SetRO( finder.IsReadOnly() );
		list.Insert( de );
	}
}

CString CDocDirNative::GetFullPath( const CString &strDir ) 
{
	CString strPath = GetPathName();
	if ( strPath.IsEmpty() && GetParentDoc() != NULL )
	{
		strPath = GetParentDoc()->GetFullPath( m_strName );
	}
	if ( strPath.Right( 1 ) != "\\" )
		strPath += "\\";
	strPath += strDir;
	int q = strPath.Find( '\\', 2 );
	if ( strPath.Right( 3 ) == "\\.." )
	{
		CString strBasePath = strPath.Left( strPath.GetLength()-3 );
		int p = strBasePath.ReverseFind( '\\' );
		if ( p > q )
			strPath = strPath.Left( p );
//			strPath.Delete( p, MAX_PATH );
		else if ( p == q )
			strPath = strPath.Left( p+1 );
//			strPath.Delete( p+1, MAX_PATH );
	}

	if ( strPath.Right( 1 ) == '\\' && strPath.GetLength() > (q+1) )
		strPath = strPath.Left( strPath.GetLength()-1 );

	return strPath;
}

class MY_AFX_COM
{
public:
	HRESULT CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
		REFIID riid, LPVOID* ppv);
	HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
};

HRESULT MY_AFX_COM::CreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
	REFIID riid, LPVOID* ppv)
{
	// find the object's class factory
	LPCLASSFACTORY pf = NULL;
	HRESULT hRes = GetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&pf);
	if (FAILED(hRes))
		return hRes;

	// call it to create the instance
	ASSERT(pf != NULL);
	hRes = pf->CreateInstance(pUnkOuter, riid, ppv);

	// let go of the factory
	pf->Release();
	return hRes;
}

CString AFXAPI AfxStringFromCLSID(REFCLSID rclsid);
BOOL AFXAPI AfxGetInProcServer(LPCTSTR lpszCLSID, CString& str);

HRESULT MY_AFX_COM::GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;
	HINSTANCE hInst = NULL;

	// find server name for this class ID

	CString strCLSID = AfxStringFromCLSID(rclsid);
	CString strServer;
	if (!AfxGetInProcServer(strCLSID, strServer))
		return REGDB_E_CLASSNOTREG;

	// try to load it
	hInst = LoadLibrary(strServer);
	if (hInst == NULL)
		return REGDB_E_CLASSNOTREG;

#pragma warning(disable:4191)
	// get its entry point
	HRESULT (STDAPICALLTYPE* pfn)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
	pfn = (HRESULT (STDAPICALLTYPE*)(REFCLSID rclsid, REFIID riid, LPVOID* ppv))
		GetProcAddress(hInst, "DllGetClassObject");
#pragma warning(default:4191)

	// call it, if it worked
	if (pfn != NULL)
		return pfn(rclsid, riid, ppv);
	return CO_E_ERRORINDLL;
}

BOOL AFXAPI ResolveShortcut(CWnd* pWnd, LPCTSTR lpszFileIn,
	LPTSTR lpszFileOut, int cchPath)
{
#if _MFC_VER >= 0x0700
	USES_CONVERSION;	// not VC6
#endif
	MY_AFX_COM com;
	IShellLink* psl;
	*lpszFileOut = 0;   // assume failure

	SHFILEINFO info;
	if ((SHGetFileInfo(lpszFileIn, 0, &info, sizeof(info),
		SHGFI_ATTRIBUTES) == 0) || !(info.dwAttributes & SFGAO_LINK))
	{
		return FALSE;
	}

	if (FAILED(com.CreateInstance(CLSID_ShellLink, NULL, IID_IShellLink,
		(LPVOID*)&psl)))
	{
		return FALSE;
	}

	IPersistFile *ppf;
	if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
	{
#if _MFC_VER < 0x0700
		int nLen = MultiByteToWideChar(CP_ACP, 0,lpszFileIn, -1, NULL, NULL);		// VC6
		LPWSTR lpszW = new WCHAR[nLen];												// VC6
		MultiByteToWideChar(CP_ACP, 0, lpszFileIn, -1, lpszW, nLen);				// VC6

		if (SUCCEEDED(ppf->Load(lpszW, STGM_READ)))									// VC6
#else
		if (SUCCEEDED(ppf->Load(T2COLE(lpszFileIn), STGM_READ)))
#endif
		{
			/* Resolve the link, this may post UI to find the link */
			if (SUCCEEDED(psl->Resolve(pWnd->GetSafeHwnd(),
				SLR_ANY_MATCH)))
			{
				HRESULT r = psl->GetPath(lpszFileOut, cchPath, NULL, 0);
				ppf->Release();
				psl->Release();
				return ( r == S_OK );
			}
		}
//		delete[] lpszW;																// VC6
		ppf->Release();
	}
	psl->Release();
	return FALSE;
}

CString CDocDirNative::GetFullPathLnk( const CString &strDir )
{
	CString strPath = GetFullPath( strDir );

	if ( strPath.Right( 4 ).CompareNoCase( _T(".lnk") ) == 0 )
	{
		TCHAR szLinkName[_MAX_PATH];
		if (ResolveShortcut(AfxGetMainWnd(), strPath, szLinkName, _MAX_PATH))
			strPath = szLinkName;
		else
		{
			AfxMessageBox( _T("Link is no file or directory"), MB_ICONWARNING );
			strPath.Empty();
		}
	}

	return strPath;
}
/*
void CDocDirNative::UpdateFile( const CString &strFullPath )
{
	// prepare GetFileModifTime() & GetFileSize
	int result = _tstati64( strFullPath, &m_fs );	// VC6
//	int result = _stat64( strFullPath, &m_fs );
	ASSERT( result == 0 );
	m_bPresent = TRUE;
}
*/
int CDocDirNative::GetSubIcon()
{
	return IconDir;
}

#ifdef _DEBUG
void CDocDirNative::AssertValid() const
{
	CDocDir::AssertValid();
	if ( m_posParent != NULL ) {
		ASSERT( m_posStorage != NULL );
	}
}
#endif
