#include "StdAfx.h"
#include "DocManFileSync.h"

#include "DocHex.h"

IMPLEMENT_DYNCREATE(CDocHex, CDocFile)

CDocHex::CDocHex(void)
{
}

CDocHex::~CDocHex(void)
{
}

void CDocHex::DeleteContents()
{
	CDocFileSync::DeleteContents();
	m_aData.RemoveAll();
	m_adwIdx.RemoveAll();
}

// CDocFileSync serialization

void CDocHex::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		StoreFile( ar.GetFile() );
	}
	else
	{
		RestoreFile( ar.GetFile() );
	}
}

void CDocHex::RestoreFile( CFile *pFile )
{
	m_aData.RemoveAll();

	TRACE0( "RestoreFile\n" );

	CFileStatus rStatus;
	pFile->GetStatus( rStatus );
	m_bReadOnly = ((rStatus.m_attribute & CFile::readOnly) == CFile::readOnly);
	CDocManFileSync* pDM = (CDocManFileSync*)AfxGetApp()->m_pDocManager;
	m_bReadOnly |= pDM->IsReadOnly();

	if ( rStatus.m_size > 0xffffff )
	{
		AfxMessageBox( _T("File is too big."), MB_ICONWARNING );
		return;
	}
	LARGE_INTEGER nReadTrans;
	nReadTrans.QuadPart = 0;
	LARGE_INTEGER nNull;
	nNull.QuadPart = 0;
	int nToRead = (int)rStatus.m_size;
	m_aData.SetSize( nToRead );

	UCHAR *pBuf = m_aData.GetData();
	int nToReadNow = 1024;
	if ( nToReadNow > nToRead )
		nToReadNow = nToRead;
	UINT nRead = pFile->Read( pBuf, nToReadNow );
	while ( nRead != 0 )
	{
		if ( m_lpProgressRoutine != NULL ) {
			nReadTrans.QuadPart += nRead;
			DWORD dwReason = CALLBACK_CHUNK_FINISHED;
			DWORD rc = (*m_lpProgressRoutine)( nNull, nReadTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
			if ( rc == PROGRESS_CANCEL )
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
		}
		pBuf += nRead;
		nToRead -= nRead;
		nToReadNow = 1024;
		if ( nToReadNow > nToRead )
			nToReadNow = nToRead;
		nRead = pFile->Read( pBuf, nToReadNow );
	}
	CreateUniqueIdx();
	TRACE0( "RestoreFile finished\n" );
}

void CDocHex::StoreFile( CFile *pFile )
{
	UCHAR *pBuf = m_aData.GetData();
	UINT nSize = (int)m_aData.GetSize();
	while ( nSize > 0 )
	{
		if ( nSize > 1024 )
		{
			pFile->Write( pBuf, 1024 );
			nSize -= 1024;
		}
		else
		{
			pFile->Write( pBuf, nSize );
			nSize = 0;
		}
	}

	TRACE0( "StoreFile finished\n" );
}

CString CDocHex::GetLineNo( int offs ) const
{
	CString strLineNo;
	strLineNo.Format( _T("%6.6X"), offs );
	return strLineNo;
}

CString CDocHex::GetDisplayLine( int offs, int len ) const
{
	CString str( _T(' '), 67 );
	LPTSTR psz = str.GetBuffer( 67 );
	int n = offs & 0xf;
	for ( int i = 0; i < len; ++i, ++n )
	{
		TCHAR szHex[4];
		BYTE b = m_aData[offs + i];
		wsprintf( szHex, _T("%2.2X"), b );
		LPTSTR pszH = psz + n*3;
		if ( n > 7 )
			++pszH;
		pszH[0] = szHex[0];
		pszH[1] = szHex[1];
		pszH = psz + n + 50;
		*pszH = b < 0x20 ? '\'' : (b >= 0x80 ? '#' : b);
	}
	str.ReleaseBuffer( 67 );
	return str;
//	return "FF FF FF FF FF FF FF FF  FF FF FF FF FF FF FF FF  AAAAAAAA AAAAAAAA"; 
}

BOOL CDocHex::SearchLine( const CString& strSearch, int offs, int len ) const
{
	int nLenS = strSearch.GetLength();
	// compare hex
	int nLenX = len + (nLenS + 2) / 3 - 1;
	if ( (offs + nLenX) > m_aData.GetCount() )
		nLenX = m_aData.GetCount() - offs;
	CString str( ' ', 3 * nLenX );
	LPTSTR psz = str.GetBuffer( 3 * nLenX );
	int i;
	for ( i = 0; i < nLenX; ++i )
	{
		TCHAR szHex[4];
		wsprintf( szHex, _T("%2.2x"), m_aData[offs + i] );
		LPTSTR pszH = psz + i*3;
		pszH[0] = szHex[0];
		pszH[1] = szHex[1];
	}
	str.ReleaseBuffer( 3 * nLenX );
	if ( str.Find( strSearch ) >= 0 ) 
		return TRUE;

	// compare text
	nLenX = len + nLenS - 1;
	if ( (offs + nLenX) > m_aData.GetCount() )
		nLenX = m_aData.GetCount() - offs;
	psz = str.GetBuffer( nLenX );
	int nLen = 0;
	for ( i = 0; i < nLenX; ++i )
	{
		int c = tolower( m_aData[offs + i] );
		if ( c != 0 ) {
			*psz++ = c;
			++nLen;
		}
	}
	str.ReleaseBuffer( nLen );
	if ( str.Find( strSearch ) >= 0 ) 
		return TRUE;
	return FALSE;
}

void CDocHex::CreateUniqueIdx()
{
	TRACE0( "CDocHex::CreateUniqueIdx()\n" );
	m_adwIdx.SetSize( 0, 128 );
	CMap<DWORD,DWORD,int,int> mapMulti;
	mapMulti.InitHashTable( 79999 );

	int pMax = (int)m_aData.GetSize()-1;
	for ( int p = 0; p < pMax; ++p )
	{
		DWORD dwCombi = *(WORD*)(m_aData.GetData() + p);
		int nIdx;
		if ( mapMulti.Lookup( dwCombi, nIdx ) )
		{
			if ( nIdx >= 0 )
			{
				m_adwIdx[nIdx] = 0xffffffff;		// mark for delete
				mapMulti[dwCombi] = -1;
			}
		}
		else
		{
			mapMulti[ dwCombi ] = (int)m_adwIdx.Add( p );
		}
	}

	for ( int n = (int)m_adwIdx.GetSize()-1; n >= 0; --n )
	{
		if ( m_adwIdx[n] == 0xffffffff )
			m_adwIdx.RemoveAt(n);
	}
}

void CDocHex::FindMatchUnique ( int &mn, int &ni,
						  CDocHex *pOther, int &mo, int &oi ) const
{
	int nMaxIdxOwn = (int)m_adwIdx.GetSize();
	for ( ; ni < nMaxIdxOwn; ++ni )
	{
		if ( (int)m_adwIdx[ni] > mn )
			break;
	}

	int nMaxIdxOther = (int)pOther->m_adwIdx.GetSize();
	for ( ; oi < nMaxIdxOther; ++oi )
	{
		if ( (int)pOther->m_adwIdx[oi] > mo )
			break;
	}

	const BYTE *pDataOwn = m_aData.GetData();
	const BYTE *pDataOther = pOther->m_aData.GetData();
	int t;
	for ( t = 0; (ni + t) < nMaxIdxOwn && (oi + t) < nMaxIdxOther; ++t )
	{
		WORD *pCombiOwnT   = (WORD*)(pDataOwn + m_adwIdx[ni+t] );
		WORD *pCombiOtherT = (WORD*)(pDataOther + pOther->m_adwIdx[oi+t] );
		if ( *pCombiOwnT == *pCombiOtherT )
		{
			mn = m_adwIdx[ni+t];
			mo = pOther->m_adwIdx[oi+t];
			return;		// found
		}
		for ( int i = 0;  i < t; ++i )
		{
			WORD *pCombiOwn = (WORD*)(pDataOwn + m_adwIdx[ni+i] );
			if ( *pCombiOwn == *pCombiOtherT )
			{
				mn = m_adwIdx[ni+i];
				mo = pOther->m_adwIdx[oi+t];
				return;
			}
			WORD *pCombiOther = (WORD*)(pDataOther + pOther->m_adwIdx[oi+i] );
			if ( *pCombiOwnT == *pCombiOther )
			{
				mn = m_adwIdx[ni+t];
				mo = pOther->m_adwIdx[oi+i];
				return;
			}
		}
	}
	for ( ; (ni + t) < nMaxIdxOwn && (oi + t) >= nMaxIdxOther; ++t )
	{
		WORD *pCombiOwnT = (WORD*)(pDataOwn + m_adwIdx[ni+t] );
		for ( int n = oi; n < nMaxIdxOther; ++n )
		{
			WORD *pCombiOther = (WORD*)(pDataOther + pOther->m_adwIdx[n] );
			if ( *pCombiOwnT == *pCombiOther )
			{
				mn = m_adwIdx[ni+t];
				mo = pOther->m_adwIdx[n];
				return;
			}
		}
	}
	for ( ; (ni + t) >= nMaxIdxOwn && (oi + t) < nMaxIdxOther; ++t )
	{
		WORD *pCombiOtherT = (WORD*)(pDataOther + pOther->m_adwIdx[oi+t] );
		for ( int n = ni; n < nMaxIdxOwn; ++n )
		{
			WORD *pCombiOwn = (WORD*)(pDataOwn + m_adwIdx[n] );
			if ( *pCombiOwn == *pCombiOtherT )
			{
				mn = m_adwIdx[n];
				mo = pOther->m_adwIdx[oi+t];
				return;
			}
		}
	}
	mn = (int)m_aData.GetSize();
	mo = (int)pOther->m_aData.GetSize();
}
