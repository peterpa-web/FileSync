#include "StdAfx.h"
#include "afxtempl.h"
#include "StringDiff.h"

#ifdef _UNICODE
	#define TCHAR2 DWORD
#else
	#define TCHAR2 WORD
#endif

CStringDiff::CStringDiff(void)
{
}

CStringDiff::~CStringDiff(void)
{
}

void CStringDiff::ResetAll(void)
{
	m_strMark.Empty();
}

void CStringDiff::Compare( const CString &strOwn, const CString &strOther, CStringDiff &strDiffOther )
{
	m_strMark = CString( (char)1, strOwn.GetLength() );
	strDiffOther.m_strMark = CString( (char)1, strOther.GetLength() );
	CDWordArray adwIdxOwn;
	CDWordArray adwIdxOther;
	GetUniqueIdx( strOwn, adwIdxOwn );
	GetUniqueIdx( strOther, adwIdxOther );

	int nMax = strOwn.GetLength();
	int oMax = strOther.GetLength();
	int ni = 0;
	int oi = 0;
	int n = 0;
	int o = 0;
	while ( n < nMax || o < oMax )
	{
		if ( n < nMax && o < oMax && strOwn[n] == strOther[o] )
		{
			m_strMark.SetAt( n++, '\0' );
			strDiffOther.m_strMark.SetAt( o++, '\0' );
			continue;
		}
		int mn = n;
		int mo = o;
		FindMatchUnique ( strOwn, mn, adwIdxOwn, ni, strOther, mo, adwIdxOther, oi );
		// backward find matching chars
		int sn = n;
		int so = o;
		int bn;
		int bo;
		for ( bn = mn - 1, bo = mo - 1; bn >= sn && bo >= so; --bn, --bo )
		{
			if ( strOwn[bn] == strOther[bo] )
			{
				m_strMark.SetAt( bn, '\0' );
				strDiffOther.m_strMark.SetAt( bo, '\0' );
			}
			else
				break;
		}
		// mark remaining chars as different
		while ( n <= bn )
			m_strMark.SetAt( n++, (char)1 );
		while ( o <= bo )
			strDiffOther.m_strMark.SetAt( o++, (char)1 );
		n = mn;
		o = mo;
	}
}

CString CStringDiff::Merge( const CString &strOwn, int nPosOwn, const CString &strOther )
{
//	m_strMark = CString( (char)1, strOwn.GetLength() );
//	strDiffOther.m_strMark = CString( (char)1, strOther.GetLength() );
	CString strRet;
//	strRet = CString( '\0', strOther.GetLength() + nSize );
	CDWordArray adwIdxOwn;
	CDWordArray adwIdxOther;
	GetUniqueIdx( strOwn, adwIdxOwn );
	GetUniqueIdx( strOther, adwIdxOther );

	int nMax = strOwn.GetLength();
	int oMax = strOther.GetLength();
	int ni = 0;
	int oi = 0;
	int n = 0;
	int o = 0;
//	int r = 0;
	int nr = 0;	// start unique area
	int or = 0;
	while ( n < nMax || o < oMax )
	{
		if ( n < nMax && o < oMax && strOwn[n] == strOther[o] )
		{
			++n;
			++o;
			continue;
		}
		int mn = n;
		int mo = o;
		FindMatchUnique ( strOwn, mn, adwIdxOwn, ni, strOther, mo, adwIdxOther, oi );
		int sn = n;
		int so = o;
		if ( nPosOwn < mn )		// handle diffs
		{
//			int bn;
//			int bo;
//			for ( bn = mn - 1, bo = mo - 1; bn >= sn && bo >= so; --bn, --bo )
//			{
//				if ( strOwn[bn] != strOther[bo] )
//					break;
//			}
			strRet = strOther.Mid( 0, or ) + strOwn.Mid( nr, mn-nr ) + strOther.Mid( mo, oMax-mo );
			return strRet;
		}
		n = mn;
		o = mo;
		nr = n;	// start unique area
		or = o;
	}

	return _T("");	// not found
}

/*
	int nStart = 0;		// merge range in own
	int nSize = 0;		// merge range size
	if ( m_strMark[nPosOwn] == '\0' )		// locate range inside strOwn
	{
		for ( int p = nPosOwn; p >= 0; --p ) {
			if ( m_strMark[p] != '\0' ) {
				nStart = p+1;
				break;
			}
		}
		for ( int p = nPosOwn; p < strOwn.GetLength(); ++p ) {
			if ( m_strMark[p] != '\0' ) {
				nSize = p - nStart;
				break;
			}
		}
		if ( nSize == 0 )
			nSize = strOwn.GetLength() - nStart;
	}
	else // marked
	{
		for ( int p = nPosOwn; p >= 0; --p ) {
			if ( m_strMark[p] == '\0' ) {
				nStart = p+1;
				break;
			}
		}
		for ( int p = nPosOwn; p < strOwn.GetLength(); ++p ) {
			if ( m_strMark[p] == '\0' ) {
				nSize = p - nStart;
				break;
			}
		}
		if ( nSize == 0 )
			nSize = strOwn.GetLength() - nStart;
	}
	int nEnd = nStart + nSize;

*/

void CStringDiff::GetUniqueIdx( const CString &strOwn, CDWordArray &adwIdx )
{
	adwIdx.SetSize( 0, 128 );
	CMap<DWORD,DWORD,int,int> mapMulti;

	int pMax = strOwn.GetLength()-1;
	for ( int p = 0; p < pMax; ++p )
	{
		DWORD dwCombi;
		dwCombi = *(TCHAR2*)((LPCTSTR)strOwn + p);
		int nIdx;
		if ( mapMulti.Lookup( dwCombi, nIdx ) )
		{
			if ( nIdx >= 0 )
			{
				adwIdx[nIdx] = 0xffffffff;		// mark for delete
				mapMulti[dwCombi] = -1;
			}
		}
		else
		{
			mapMulti[ dwCombi ] = (int)adwIdx.Add( p );
		}
	}

	for ( int n = (int)adwIdx.GetSize()-1; n >= 0; --n )
	{
		if ( adwIdx[n] == 0xffffffff )
			adwIdx.RemoveAt(n);
	}
}

void CStringDiff::FindMatchUnique ( const CString &strOwn, int &mn, CDWordArray &adwIdxOwn, int &ni,
						  const CString &strOther, int &mo, CDWordArray &adwIdxOther, int &oi )
{
	int nMaxIdxOwn = (int)adwIdxOwn.GetSize();
	for ( ; ni < nMaxIdxOwn; ++ni )
	{
		if ( (int)adwIdxOwn[ni] > mn )
			break;
	}

	int nMaxIdxOther = (int)adwIdxOther.GetSize();
	for ( ; oi < nMaxIdxOther; ++oi )
	{
		if ( (int)adwIdxOther[oi] > mo )
			break;
	}

	int t;
	for ( t = 0; (ni + t) < nMaxIdxOwn && (oi + t) < nMaxIdxOther; ++t )
	{
		TCHAR2 *pCombiOwnT   = (TCHAR2*)((LPCTSTR)strOwn + adwIdxOwn[ni+t] );
		TCHAR2 *pCombiOtherT = (TCHAR2*)((LPCTSTR)strOther + adwIdxOther[oi+t] );
		if ( *pCombiOwnT == *pCombiOtherT )
		{
			mn = adwIdxOwn[ni+t];
			mo = adwIdxOther[oi+t];
			return;		// found
		}
		for ( int i = 0;  i < t; ++i )
		{
			TCHAR2 *pCombiOwn = (TCHAR2*)((LPCTSTR)strOwn + adwIdxOwn[ni+i] );
			if ( *pCombiOwn == *pCombiOtherT )
			{
				mn = adwIdxOwn[ni+i];
				mo = adwIdxOther[oi+t];
				return;
			}
			TCHAR2 *pCombiOther = (TCHAR2*)((LPCTSTR)strOther + adwIdxOther[oi+i] );
			if ( *pCombiOwnT == *pCombiOther )
			{
				mn = adwIdxOwn[ni+t];
				mo = adwIdxOther[oi+i];
				return;
			}
		}
	}
	for ( ; (ni + t) < nMaxIdxOwn && (oi + t) >= nMaxIdxOther; ++t )
	{
		TCHAR2 *pCombiOwnT = (TCHAR2*)((LPCTSTR)strOwn + adwIdxOwn[ni+t] );
		for ( int n = oi; n < adwIdxOther.GetSize(); ++n )
		{
			TCHAR2 *pCombiOther = (TCHAR2*)((LPCTSTR)strOther + adwIdxOther[n] );
			if ( *pCombiOwnT == *pCombiOther )
			{
				mn = adwIdxOwn[ni+t];
				mo = adwIdxOther[n];
				return;
			}
		}
	}
	for ( ; (ni + t) >= nMaxIdxOwn && (oi + t) < nMaxIdxOther; ++t )
	{
		TCHAR2 *pCombiOtherT = (TCHAR2*)((LPCTSTR)strOther + adwIdxOther[oi+t] );
		for ( int n = ni; n < adwIdxOwn.GetSize(); ++n )
		{
			TCHAR2 *pCombiOwn = (TCHAR2*)((LPCTSTR)strOwn + adwIdxOwn[n] );
			if ( *pCombiOwn == *pCombiOtherT )
			{
				mn = adwIdxOwn[n];
				mo = adwIdxOther[oi+t];
				return;
			}
		}
	}
	mn = strOwn.GetLength();
	mo = strOther.GetLength();
}

