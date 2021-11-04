#include "StdAfx.h"
#include <shlobj.h>
#include <shlwapi.h>
#include "pidl.h"

LPMALLOC CPIDL::s_lpMalloc = NULL;
//	s_lpMalloc->Release();  not provided    

CPIDL::CPIDL(void)
{
	if ( s_lpMalloc == NULL )
	{
		if (::SHGetMalloc(&s_lpMalloc) != NOERROR)
		{
			ASSERT( FALSE );
		}
	}

	m_pidl = NULL;
}

CPIDL::~CPIDL(void)
{
	Empty();
}

void CPIDL::Empty()
{
	if ( m_pidl != NULL )
	{
		s_lpMalloc->Free(m_pidl);
		m_pidl = NULL;
	}
}

void CPIDL::operator =( const CPIDL& pidl )
{
	Empty();
	if ( pidl.m_pidl == NULL )
		return;

	SIZE_T cb = s_lpMalloc->GetSize(pidl.m_pidl);
	m_pidl = (LPITEMIDLIST)s_lpMalloc->Alloc(cb);
	if ( m_pidl == NULL )
		AfxThrowMemoryException();
	CopyMemory( m_pidl, pidl, cb );
}

STDAPI PIDLtoString(LPCITEMIDLIST pidl, HWND hwnd, LPTSTR pszDisplName, UINT cchDisplName)
{
	*pszDisplName = 0;

    IShellFolder* psf;
    LPCITEMIDLIST pidlChild = NULL;

    HRESULT hr = SHBindToParent(pidl, IID_IShellFolder, (void **)&psf, &pidlChild);
    if (SUCCEEDED(hr))
    {
		STRRET strDisplName;
		IShellLink *psl = NULL;
        hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, IID_IShellLink, NULL, (void **)&psl);
		if (SUCCEEDED(hr))		// is link
		{
			LPITEMIDLIST pidlTarget = NULL;
			hr = psl->GetIDList(&pidlTarget);
			if (SUCCEEDED(hr))
			{
				hr = psf->GetDisplayNameOf( pidlTarget, SHGDN_FORPARSING, &strDisplName );
				if (SUCCEEDED(hr))
					hr = StrRetToBuf( &strDisplName, pidlTarget, pszDisplName, cchDisplName );
				else
				{
					IShellFolder* ptf;
					LPCITEMIDLIST pidlTPar = NULL;
					hr = SHBindToParent(pidlTarget, IID_IShellFolder, (void **)&ptf, &pidlTPar);
					if (SUCCEEDED(hr))
					{
						hr = ptf->GetDisplayNameOf( pidlTPar, SHGDN_FORPARSING, &strDisplName );
						if (SUCCEEDED(hr))
							hr = StrRetToBuf( &strDisplName, pidlTPar, pszDisplName, cchDisplName );
						ptf->Release();
					}
				}
				CoTaskMemFree(pidlTarget);
			}
			psl->Release();
		}
		else					// no link
		{
			hr = psf->GetDisplayNameOf( pidlChild, SHGDN_FORPARSING, &strDisplName );
			if (SUCCEEDED(hr))
				hr = StrRetToBuf( &strDisplName, pidlChild, pszDisplName, cchDisplName );
		}
        psf->Release();
    }
    return hr;
}

CString CPIDL::ToString(LPITEMIDLIST pidl)
{
	TCHAR szDisplName[MAX_PATH];
	if ( PIDLtoString(pidl, NULL, szDisplName, sizeof(szDisplName) ) != S_OK )
	{
		ASSERT( FALSE );
		return _T("");
	}
	return szDisplName;
}

BOOL CPIDL::FromString( LPTSTR path )
{
	Empty();

	USES_CONVERSION;
	IShellFolder *psfDeskTop = NULL;
	ULONG chEaten;
	HRESULT hr;

	if ( _tcsncmp(path, _T("\\\\"), 2) == 0 && _tcschr(path, '$') != NULL)	// hidden net path
		return FromCSIDL(CSIDL_NETWORK);

	hr = SHGetDesktopFolder(&psfDeskTop);
	if ( hr != NOERROR )
		return FALSE; // failed to get desktopfolder

	hr = psfDeskTop->ParseDisplayName(NULL, NULL, T2OLE(path), &chEaten, &m_pidl, NULL);
	psfDeskTop->Release();
	if ( hr == S_OK && _tcsncmp(path, _T("\\\\"), 2) == 0 )
	{
		CString str = ToString( m_pidl );
		if ( str.CompareNoCase( path ) == 0 )
			return TRUE;
		// otherwise present network folder:
		return FromCSIDL(CSIDL_NETWORK);
	}
	return ( hr == S_OK );
}

BOOL CPIDL::FromCSIDL( int csidl )
{
	Empty();

	HRESULT hr = SHGetSpecialFolderLocation( NULL, csidl, &m_pidl );
	return ( hr == S_OK );
}
