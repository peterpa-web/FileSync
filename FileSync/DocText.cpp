#include "StdAfx.h"
#include "StringDiff.h"
#include "DocManFileSync.h"
#include "ConvertUTF.h"

#include "DocText.h"

CDocText::LineData::LineData()
{
	dwHash = 0;
	nLine = 0;
	bUnique = FALSE;
	posOther = NULL;
}

void CDocText::LineData::GetWords( CStringList &lstr ) const
{
	lstr.RemoveAll();
	CString strWord;
	int n;
	for ( n = 0; n < strRaw.GetLength(); ++n )
	{
		TCHAR c = strRaw[n];
		if ( (c >= 'A' && c <= 'Z') ||
			 (c >= 'a' && c <= 'z') ||
			 c == '_' )
		{
			strWord.AppendChar( c );
			continue;
		}
		if ( strWord.IsEmpty() )
			continue;	// skip char

		if ( (c >= '0' && c <= '9') )
		{
			strWord.AppendChar( c );
			continue;
		}
		// other char
		if ( strWord.GetLength() >= 3 )
			lstr.AddTail( strWord );
		strWord.Empty();
	}
	if ( strWord.GetLength() >= 3 )
		lstr.AddTail( strWord );
	if ( lstr.GetCount() != 0 )
		return;

	// otherwise find special patterns like ###
	strWord.Empty();
	for ( n = 0; n < strRaw.GetLength(); ++n )
	{
		TCHAR c = strRaw[n];
		if ( c >= '!' && c <= '@') {
			if ( strWord.IsEmpty() )
				strWord.AppendChar( c );
			else if ( c == strWord.GetAt(0) )
				strWord.AppendChar( c );
			else {
				strWord.Empty();
				strWord.AppendChar( c );
			}
			if ( strWord.GetLength() == 3 ) {
				lstr.AddTail( strWord );
			}
		}
		else
			strWord.Empty();
	}
}

void CDocText::LineData::GetCombinedWords( CStringList &lstr, DWORD dwMode /* = 0xFFFF */ ) const
{
	lstr.RemoveAll();
	CString strComb;
	CStringList lstrWords;
	GetWords( lstrWords );
	if ( lstrWords.GetSize() == 0 )
		return;

	if ( lstrWords.GetSize() > 20 ) {
		TRACE1( "CDocText::LineData::GetCombinedWords >20 line=%d\n", nLine );
		dwMode = 4;
	}

	// combine the n words ...
	if ( (dwMode & 1) != 0 )
	{
		POSITION pos = lstrWords.GetHeadPosition();
		while ( pos != NULL )
		{
			const CString &strWord = lstrWords.GetNext( pos );
			if ( !strComb.IsEmpty() )
				strComb.AppendChar( '|' );
			strComb.Append( strWord );
		}
		lstr.AddTail( strComb );
	}

	// combine all n-1 words ...
	if ( (dwMode & 2) != 0 )
	{
		for ( int n = 0; n < lstrWords.GetSize(); ++n )
		{
			strComb.Empty();
			int i = 0;
			POSITION pos = lstrWords.GetHeadPosition();
			while ( pos != NULL )
			{
				const CString &strWord = lstrWords.GetNext( pos );
				if ( i++ == n )
					continue;
				if ( !strComb.IsEmpty() )
					strComb.AppendChar( '|' );
				strComb.Append( strWord );
			}
			if ( !strComb.IsEmpty() )
				lstr.AddTail( strComb );
		}
	}

	// build all word pairs ...
	if ( lstrWords.GetSize() < 4 )
		return;

	if ( (dwMode & 4) != 0 )
	{
		POSITION pos = lstrWords.GetHeadPosition();
		strComb = lstrWords.GetNext( pos );
		while ( pos != NULL )
		{
			const CString &strWord = lstrWords.GetNext( pos );
			strComb.AppendChar( '|' );
			strComb.Append( strWord );
			lstr.AddTail( strComb );
			strComb = strWord;
		}
	}
}

// #####################

// keeping single line data position OR list of line data positions

 CDocText::CMapElem::CMapElem()
 {
	 m_posLineData = NULL;
	 m_pPosList = NULL;
 }

 CDocText::CMapElem::CMapElem( POSLINE pos)
 {
	 ASSERT(POSLINE_ISVALID(pos));
	 m_posLineData = pos;
	 m_pPosList = NULL;
 }

 CDocText::CMapElem::~CMapElem()
 {
	 m_posLineData = NULL;
	 delete m_pPosList;
 }

 POSLINE CDocText::CMapElem::GetFirstLineData() 
 {
	 if ( m_posLineData != NULL )
		return m_posLineData; 
 	 if ( m_pPosList == NULL || m_pPosList->IsEmpty() )
		return NULL;
	 return m_pPosList->GetHead();
}


 void CDocText::CMapElem::Add( POSLINE pos )
 {
	 ASSERT(POSLINE_ISVALID(pos));
	 if ( m_posLineData == NULL &&
			( m_pPosList == NULL || m_pPosList->IsEmpty() ) ) {
		 m_posLineData = pos;
	 }
	 else
	 {
		 if ( m_pPosList == NULL )
			 m_pPosList = new CPosList();
		 if ( m_posLineData != NULL )
		 {
			 m_pPosList->AddTail( m_posLineData );
			 m_posLineData = NULL;
		 }
		 m_pPosList->AddTail( pos );
	 }
 }

 void CDocText::CMapElem::Remove (POSLINE pos )
 {
	 ASSERT(POSLINE_ISVALID(pos));
 	 if ( m_posLineData == pos ) {
		 m_posLineData = NULL;
		 ASSERT( m_pPosList == NULL || m_pPosList->IsEmpty() );
	 }
	 else
	 {
		 ASSERT( m_pPosList != NULL );
		 POSITION posFind = m_pPosList->Find(pos);
		 if ( posFind != NULL )
			 m_pPosList->RemoveAt( posFind );
		 if ( m_pPosList->GetCount() == 1 ) {	// move to m_posLineData
			 m_posLineData = m_pPosList->GetHead();
			 m_pPosList->RemoveAll();
		 }
	 }
}


// #####################

IMPLEMENT_DYNCREATE(CDocText, CDocFile)

LPCTSTR CDocText::s_pszEncodingDefault		= _T("default");
LPCTSTR CDocText::s_pszEncodingUnicode		= _T("Unicode");
LPCTSTR CDocText::s_pszEncodingUnicodeRev	= _T("Unicode Rev");
LPCTSTR CDocText::s_pszEncodingUTF8			= _T("UTF-8");
LPCTSTR CDocText::s_pszEncodingXUTF8		= _T("XML UTF-8");

CDocText::CDocText(void) : m_listLines(500), m_mapHashToLines(500)
{
	m_nMaxLineLen = 0;
	m_nTabExpand = 4;
	m_bCompactSpace = FALSE;
	m_bUnixFormat = FALSE;
	m_strEncoding = s_pszEncodingDefault;
}

CDocText::~CDocText(void)
{
}

void CDocText::DeleteContents()
{
	CDocFileSync::DeleteContents();
	// test
	POSLINE pos = GetFirstLine();
	while ( POSLINE_ISVALID(pos) )
	{
		LineData &ld = GetLineDataAt( pos );
		ld.strRaw.Empty();
		ld.strDiff.ResetAll();
		ld.strExp.Empty();
		GetNextLine( pos );
	}
	// end test
	m_listLines.RemoveAll();	
	m_mapHashToLines.RemoveAll();
	m_nMaxLineLen = 0;
	m_nTabExpand = 4;		// 20100428
	m_bCompactSpace = FALSE;
	m_bUnixFormat = FALSE;
	m_strEncoding = s_pszEncodingDefault;
}

static ConversionResult CopyUnicode8to16 (
	const char** sourceStart, const char* sourceEnd, 
	WCHAR** targetStart, WCHAR* targetEnd) {
    ConversionResult result = conversionOK;
    const char* source = *sourceStart;
    WCHAR* target = *targetStart;
    while (source < sourceEnd) {
		WCHAR ch = 0;
		if (source + 1 >= sourceEnd) {
			result = sourceExhausted; break;
		}
		ch = *((WCHAR*)source);
		source += 2;
		if (target >= targetEnd) {
			source -= 2; /* Back up source pointer! */
			result = targetExhausted; break;
		}
		*target++ = ch;
    }
    *sourceStart = source;
    *targetStart = target;
    return result;
}

static ConversionResult CopyUnicode16to8 (
	const WCHAR** sourceStart, const WCHAR* sourceEnd, 
	char** targetStart, char* targetEnd) {
    ConversionResult result = conversionOK;
    const WCHAR* source = *sourceStart;
    char* target = *targetStart;
    while (source < sourceEnd) {
		WCHAR ch = *source++;
		if (target+2 > targetEnd) {
			--source; /* Back up source pointer! */
			result = targetExhausted; break;
		}
		*target++ = (char)(ch & 0xff);	// low byte
		*target++ = (char)((ch >> 8) & 0xff);	// high byte
    }
    *sourceStart = source;
    *targetStart = target;
    return result;
}

// CDocFileSync serialization

void CDocText::Serialize(CArchive& ar)
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

void CDocText::RestoreFile( CFile *pFile )
{
//	CWaitCursor waitCursor;

//	SetModifiedFlag( FALSE );
	m_listLines.RemoveAll();
	m_bUnixFormat = FALSE;
	m_strEncoding = s_pszEncodingDefault;

	TRACE0( "CDocText::RestoreFile\n" );

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
	int nToRead = (int)rStatus.m_size;

	m_nMaxLineLen = 0;
	CString strLine;

	int nLine = 0;
	int nLinesCR = 0;
	LARGE_INTEGER nReadTrans;
	nReadTrans.QuadPart = 0;
	LARGE_INTEGER nNull;
	nNull.QuadPart = 0;

#define BUF_SZ 4096
	char szBuf[BUF_SZ+1];
	int nToReadNow = BUF_SZ;
	if ( nToReadNow > nToRead )
		nToReadNow = nToRead;
	UINT nRead = pFile->Read( szBuf, nToReadNow );
	if ( m_lpProgressRoutine != NULL ) {
		nReadTrans.QuadPart += nRead;
		DWORD dwReason = CALLBACK_CHUNK_FINISHED;
		DWORD rc = (*m_lpProgressRoutine)( nNull, nReadTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
		if ( rc == PROGRESS_CANCEL )
			return;
	}
	if ( nRead >= 3 )
	{
		// check for translation type CP_ACP or CP_THREAD_ACP, CP_UTF8, 
		// for UTF-8: ef bb bf (use MultiByteToWideChar), AtlUnicodeToUTF8
		// Byte Order Mark (BOM)
		if ( szBuf[0] == (char)0xef && szBuf[1] == (char)0xbb && szBuf[2] == (char)0xbf )
		{
			m_strEncoding = s_pszEncodingUTF8;
		}
		// use IsTextUnicode()
		// for Unicode: ff fe
		else if ( szBuf[0] == (char)0xff && szBuf[1] == (char)0xfe )
		{
			m_strEncoding = s_pszEncodingUnicode;
//			AfxMessageBox( m_strPathName + _T("\nUnicode is not supported now."), MB_ICONWARNING );
//			return;
		}
		// for Unicode big endian: fe ff
		else if ( szBuf[0] == (char)0xfe && szBuf[1] == (char)0xff )
		{
			m_strEncoding = s_pszEncodingUnicodeRev;
			AfxMessageBox( m_strPathName + _T("\nUnicode big endian is not supported now."), MB_ICONWARNING );
			return;
		}
		// check for XML encoding like <?xml version="1.0" encoding="UTF-8"?>
		else if ( szBuf[0] == '<' && szBuf[1] == '?' && szBuf[2] == 'x' ) {
			szBuf[nRead] = '\0';
			char *pEOL = strchr( szBuf, '\n' );
			if ( pEOL != NULL ) {
				char *pEnc = strstr( szBuf, "encoding=\"UTF-8\"" );
				if ( pEnc != NULL && pEnc < pEOL )
					m_strEncoding = s_pszEncodingXUTF8;
			}
		}
	}
	if ( m_strEncoding == s_pszEncodingDefault )
	{
		while ( nRead != 0 )
		{
			szBuf[nRead] = '\0';
			char *s = szBuf;
			char *p = strchr( szBuf, '\n' );
			while ( p != NULL )
			{
				int nLen = (int)(p - s);
				strLine += CString( s, nLen );
				StoreLine( nLine, nLinesCR, strLine );
				strLine.Empty();
				s = p + 1;
				p = strchr( s, '\n' );
			}
			strLine += s;

			nToRead -= nRead;
			nToReadNow = BUF_SZ;
			if ( nToReadNow > nToRead )
				nToReadNow = nToRead;
			nRead = pFile->Read( szBuf, nToReadNow );
			if ( m_lpProgressRoutine != NULL ) {
				nReadTrans.QuadPart += nRead;
				DWORD dwReason = CALLBACK_CHUNK_FINISHED;
				DWORD rc = (*m_lpProgressRoutine)( nNull, nReadTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
				if ( rc == PROGRESS_CANCEL )
					AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}
		if ( !strLine.IsEmpty() )
		{
			StoreLine( nLine, nLinesCR, strLine );
		}
	}
	else if ( m_strEncoding == s_pszEncodingUTF8 || m_strEncoding == s_pszEncodingXUTF8 )
	{
		WCHAR szBuf16[BUF_SZ+1];
		const UTF8* sourceStart = (UTF8*)szBuf+3;
		UTF16* targetStart = (UTF16*)szBuf16;

		if ( m_strEncoding == s_pszEncodingXUTF8 ) {
			const UTF8* pC = NULL;
			for ( pC = sourceStart; pC < (UTF8*)szBuf+nRead; ++pC ) {
				if ( *pC == '\n' ) {
					CString str( szBuf, ((char*)pC)-szBuf );
					StoreLine( nLine, nLinesCR, str );
					sourceStart = pC + 1;
					break;
				}
			}
			if ( sourceStart != pC + 1 ) {
				ASSERT( FALSE );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
		}

		while ( nRead != 0 )
		{
			ConversionResult res = ConvertUTF8toUTF16 (
				&sourceStart, (const UTF8*)(szBuf+nRead), 
				&targetStart, (UTF16*)(szBuf16+BUF_SZ), lenientConversion );
			if ( res != conversionOK && res != sourceExhausted )
			{
				ASSERT( FALSE );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
			*targetStart = 0;
			wchar_t *s = szBuf16;
			wchar_t *p = wcschr( szBuf16, '\n' );
			while ( p != NULL )
			{
				int nLen = (int)(p - s);
				strLine += CString( s, nLen );
				StoreLine( nLine, nLinesCR, strLine );
				strLine.Empty();
				s = p + 1;
				p = wcschr( s, '\n' );
			}
			strLine += s;

			if ( res == sourceExhausted )
			{
				int nExtra = nRead - (sourceStart - (UTF8*)szBuf);
				ASSERT( nExtra == 1 );
				nToRead -= (nRead - nExtra);
				nToReadNow = BUF_SZ - nExtra;
				memcpy( szBuf, sourceStart, nExtra );
				if ( nToReadNow > nToRead )
					nToReadNow = nToRead;
				nRead = pFile->Read( szBuf+nExtra, nToReadNow );
				if ( nRead == 0 )
					AfxThrowFileException( CFileException::endOfFile, -1, m_strPathName );
				if ( m_lpProgressRoutine != NULL ) {
					nReadTrans.QuadPart += nRead;
					DWORD dwReason = CALLBACK_CHUNK_FINISHED;
					DWORD rc = (*m_lpProgressRoutine)( nNull, nReadTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
					if ( rc == PROGRESS_CANCEL )
						AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
				}
				nRead += nExtra;
			}
			else
			{
				nToRead -= nRead;
				nToReadNow = BUF_SZ;
				if ( nToReadNow > nToRead )
					nToReadNow = nToRead;
				nRead = pFile->Read( szBuf, nToReadNow );
			}
			sourceStart = (UTF8*)szBuf;
			targetStart = (UTF16*)szBuf16;
		}
		if ( !strLine.IsEmpty() )
		{
			StoreLine( nLine, nLinesCR, strLine );
		}
	}
	else if ( m_strEncoding == s_pszEncodingUnicode )
	{
		WCHAR szBuf16[BUF_SZ+1];
		const char* sourceStart = szBuf+2;
		WCHAR* targetStart = szBuf16;

		while ( nRead != 0 )
		{
			ConversionResult res = CopyUnicode8to16 (
				&sourceStart, (szBuf+nRead), 
				&targetStart, (szBuf16+BUF_SZ) );
			if ( res != conversionOK && res != sourceExhausted )
			{
				ASSERT( FALSE );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
			*targetStart = 0;
			wchar_t *s = szBuf16;
			wchar_t *p = wcschr( szBuf16, '\n' );
			while ( p != NULL )
			{
				int nLen = (int)(p - s);
				strLine += CString( s, nLen );
				StoreLine( nLine, nLinesCR, strLine );
				strLine.Empty();
				s = p + 1;
				p = wcschr( s, '\n' );
			}
			strLine += s;

			if ( res == sourceExhausted )
			{
				int nExtra = nRead - (sourceStart - szBuf);
				ASSERT( nExtra == 1 );
				nToRead -= (nRead - nExtra);
				nToReadNow = BUF_SZ - nExtra;
				memcpy( szBuf, sourceStart, nExtra );
				if ( nToReadNow > nToRead )
					nToReadNow = nToRead;
				nRead = pFile->Read( szBuf+nExtra, nToReadNow );
				if ( nRead == 0 )
					AfxThrowFileException( CFileException::endOfFile, -1, m_strPathName );
				nRead += nExtra;
			}
			else
			{
				nToRead -= nRead;
				nToReadNow = BUF_SZ;
				if ( nToReadNow > nToRead )
					nToReadNow = nToRead;
				nRead = pFile->Read( szBuf, nToReadNow );
				if ( m_lpProgressRoutine != NULL ) {
					nReadTrans.QuadPart += nRead;
					DWORD dwReason = CALLBACK_CHUNK_FINISHED;
					DWORD rc = (*m_lpProgressRoutine)( nNull, nReadTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
					if ( rc == PROGRESS_CANCEL )
						AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
				}
			}
			sourceStart = szBuf;
			targetStart = szBuf16;
		}
		if ( !strLine.IsEmpty() )
		{
			StoreLine( nLine, nLinesCR, strLine );
		}
	}
	m_bUnixFormat = ( nLinesCR <= ( nLine / 2 ) );
	UpdateInternalLines();

	TRACE2( "RestoreFile %d lines Unix=%d\n", nLine, m_bUnixFormat );
}

void CDocText::StoreLine( int &nLine, int &nLinesCR, CString strLine )
{
	if ( strLine.Right(1) == _T("\r") )
	{
		int nLen = strLine.GetLength();
		TCHAR *pLine = strLine.GetBuffer(nLen);
		strLine.ReleaseBuffer( nLen - 1 );
		++nLinesCR;
	}
	LineData ld;
	ld.strRaw = strLine;
	ld.nLine = ++nLine;
	m_listLines.AddTail( ld );
}

void CDocText::StoreFile( CFile *pFile )
{
	LARGE_INTEGER nLinesTrans;
	nLinesTrans.QuadPart = 0;
	LARGE_INTEGER nNull;
	nNull.QuadPart = 0;

	POSLINE pos = GetFirstLine();
	if ( m_strEncoding == s_pszEncodingDefault )
	{
		int nLine;
		for ( nLine = 0; POSLINE_ISVALID(pos); ++nLine )
		{
			LineData &ld = GetLineDataAt( pos );
			GetNextLine( pos );
			CStringA strLine( ld.strRaw );
			if ( !strLine.IsEmpty() )
				pFile->Write( (LPCSTR)strLine, strLine.GetLength() );
			if ( m_bUnixFormat )
				pFile->Write( "\n", 1 );
			else
				pFile->Write( "\r\n", 2 );
			if ( m_lpProgressRoutine != NULL ) {
				++nLinesTrans.QuadPart;
				DWORD dwReason = CALLBACK_CHUNK_FINISHED;
				DWORD rc = (*m_lpProgressRoutine)( nNull, nLinesTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
				if ( rc == PROGRESS_CANCEL )
					return;
			}
		}
		TRACE1( "StoreFile %d lines\n", nLine );
	}
	else if ( m_strEncoding == s_pszEncodingUTF8 || m_strEncoding == s_pszEncodingXUTF8 )
	{
		if ( m_strEncoding == s_pszEncodingUTF8 )
			pFile->Write( "\xef\xbb\xbf", 3 );
		else {
			LineData &ld = GetLineDataAt( pos );
			if ( ld.strRaw.Find(_T("encoding=\"UTF-8\"")) < 5 ) {
				pFile->Write( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 38 );
				if ( m_bUnixFormat )
					pFile->Write( "\n", 1 );
				else
					pFile->Write( "\r\n", 2 );
			}
		}

		UTF8* pBuf = new UTF8[2*m_nMaxLineLen];
		int nLine;
		for ( nLine = 0; POSLINE_ISVALID(pos); ++nLine )
		{
			LineData &ld = GetLineDataAt( pos );
			GetNextLine( pos );
			ASSERT( ld.strRaw.GetLength() <= m_nMaxLineLen );
			const UTF16* sourceStart = (UTF16*)(LPCTSTR)ld.strRaw;
			const UTF16* sourceEnd = sourceStart+ld.strRaw.GetLength();
			UTF8* targetStart = pBuf;
			UTF8* targetEnd = pBuf + 2*m_nMaxLineLen;
			ConversionResult res = ConvertUTF16toUTF8( 
				&sourceStart, sourceEnd, 
				&targetStart, targetEnd, lenientConversion );
			if ( res != conversionOK )
			{
				ASSERT( FALSE );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
			if ( targetStart != pBuf )
				pFile->Write( pBuf, targetStart - pBuf );
			if ( m_bUnixFormat )
				pFile->Write( "\n", 1 );
			else
				pFile->Write( "\r\n", 2 );
			if ( m_lpProgressRoutine != NULL ) {
				++nLinesTrans.QuadPart;
				DWORD dwReason = CALLBACK_CHUNK_FINISHED;
				DWORD rc = (*m_lpProgressRoutine)( nNull, nLinesTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
				if ( rc == PROGRESS_CANCEL )
					return;
			}
		}
		delete [] pBuf;
		TRACE1( "StoreFile UTF8 %d lines\n", nLine );
	}
	else if ( m_strEncoding == s_pszEncodingUnicode )
	{
//		AfxMessageBox( m_strPathName + _T("\nUnicode is not supported now."), MB_ICONWARNING );
		pFile->Write( "\xff\xfe", 2 );

		char* pBuf = new char[2*m_nMaxLineLen];
		int nLine;
		for ( nLine = 0; POSLINE_ISVALID(pos); ++nLine )
		{
			LineData &ld = GetLineDataAt( pos );
			GetNextLine( pos );
			ASSERT( ld.strRaw.GetLength() <= m_nMaxLineLen );
			const WCHAR* sourceStart = (const WCHAR*)ld.strRaw;
			const WCHAR* sourceEnd = sourceStart+ld.strRaw.GetLength();
			char* targetStart = pBuf;
			char* targetEnd = pBuf + 2*m_nMaxLineLen;
			ConversionResult res = CopyUnicode16to8( 
				&sourceStart, sourceEnd, 
				&targetStart, targetEnd );
			if ( res != conversionOK )
			{
				ASSERT( FALSE );
				AfxThrowFileException( CFileException::genericException, -1, m_strPathName );
			}
			if ( targetStart != pBuf )
				pFile->Write( pBuf, targetStart - pBuf );
			if ( m_bUnixFormat )
				pFile->Write( "\n\0", 2 );
			else
				pFile->Write( "\r\0\n\0", 4 );
			if ( m_lpProgressRoutine != NULL ) {
				++nLinesTrans.QuadPart;
				DWORD dwReason = CALLBACK_CHUNK_FINISHED;
				DWORD rc = (*m_lpProgressRoutine)( nNull, nLinesTrans, nNull, nNull, 0, dwReason, NULL, NULL, m_pProgressMan );
				if ( rc == PROGRESS_CANCEL )
					return;
			}
		}
		delete [] pBuf;
		TRACE1( "StoreFile Unicode %d lines\n", nLine );
	}
	else if ( m_strEncoding == s_pszEncodingUnicodeRev )
	{
		AfxMessageBox( m_strPathName + _T("\nUnicode big endian is not supported now."), MB_ICONWARNING );
		return;
	}
}

POSLINE CDocText::InsertInternalLines( POSLINE &posStart, const CStringArray &astrLines )
{
	if ( astrLines.GetSize() < 1 )
		return NULL;

	posStart = InsertInternalLine( posStart, astrLines[0] );
	POSLINE posEnd = posStart;
	GetNextLine( posEnd );
	for ( int n = 1; n < astrLines.GetSize(); ++n )
	{
		posEnd = InsertInternalLine( posEnd, astrLines[n] );
		GetNextLine( posEnd );
	}
	Renumber( posStart );
	return posEnd;
}

POSLINE CDocText::InsertInternalLine( POSLINE pos, const CString &strLine )
{
	// needs Renumber() !
	LineData ld;		// to be filled by UpdateInternalLine()
	ld.strRaw = strLine;
	if ( !POSLINE_ISVALID(pos) )
		pos = (POSLINE)m_listLines.AddTail( ld );
	else {
		ASSERT(POSLINE_ISVALID(pos));
		pos = (POSLINE)m_listLines.InsertBefore( (POSITION)pos, ld );
	}

	UpdateInternalLine( pos );
	SetModifiedFlag();
	return pos;
}

void CDocText::UpdateInternalLine( POSLINE pos )
{
	ASSERT(POSLINE_ISVALID(pos));
	LineData &ld = GetLineDataAt( pos );
	CString strLine = ld.strRaw;
	if ( m_bCompactSpace )
	{
		strLine.TrimLeft( _T(" \t") );	// VC6
		strLine.TrimRight( _T(" \t") );
//		strLine = strLine.Trim( " \t" );
	}
	// expand tabs ...
	int p = -1;
	int nTabs = 0;
	while ( (p = strLine.Find( '\t', p+1 )) >= 0 )
		++nTabs;

	if ( nTabs > 0 )
	{
		p = 0;
		int l = strLine.GetLength();
		CString strLineEx;
		LPCTSTR ps = strLine;
		int lExMax = l + nTabs * m_nTabExpand;
		TCHAR *pd = strLineEx.GetBufferSetLength( lExMax );

		int lEx = 0;
		for ( int n = 0; n < l; ++n )
		{
			if ( *ps == '\t' )
			{
				int nSpaces = m_nTabExpand - (lEx % m_nTabExpand);
				for ( int ns = nSpaces; ns > 0; --ns )
				{
					*pd++ = ' ';
				}
				lEx += nSpaces;
			}
			else
			{
				++lEx;
				*pd++ = *ps;
			}
			ASSERT( lEx <= lExMax );
			++ps;
		}

		strLineEx.ReleaseBuffer( lEx );
		strLine = strLineEx;
	}
	if ( m_bCompactSpace )
	{
		int p = 0;
		while ( (p = strLine.Find( _T("  "), p )) >= 0 )
		{
			strLine = strLine.Left(p) + strLine.Mid( p+1 );
		}
	}

	ld.strExp = strLine;
	DWORD dwHash = GetHash( strLine );
	ld.dwHash = dwHash;
	CMapElem el;
	if ( m_mapHashToLines.Lookup( dwHash, el ) )
	{
		POSLINE posLD = el.GetFirstLineData();
		if ( posLD == NULL) {
			ld.bUnique = TRUE;
		}
		else {
			LineData &ldn = GetLineDataAt( posLD );
			ldn.bUnique = FALSE;
			ld.bUnique = FALSE;
		}
		el.Add( pos );
		m_mapHashToLines[ dwHash ] = el;
		el.Free();
	}
	else
	{
		CMapElem elNew( pos );
		m_mapHashToLines[ dwHash ] = elNew;
		ld.bUnique = TRUE;
	}

	if ( strLine.GetLength() > m_nMaxLineLen )
		m_nMaxLineLen = strLine.GetLength();
}

void CDocText::UpdateInternalLines()
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
	int i = 0;
#endif
	CWaitCursor waitCursor;

	TRACE0( "CDocText::UpdateInternalLines\n" );
	m_nMaxLineLen = 0;

	m_mapHashToLines.RemoveAll();
	m_mapHashToLines.InitHashTable( (int)m_listLines.GetCount() + 17 );
	POSLINE pos = GetFirstLine();
	while ( POSLINE_ISVALID(pos) )
	{
		UpdateInternalLine( pos );
		GetNextLine( pos );
#ifdef _DEBUG
		if ( (++i & 0xff) == 0 ) {
			TRACE1( "CDocText::UpdateInternalLines() line=%d\n", i );
		}
#endif
	}
	Renumber( POSLINE_TOP );
	TRACE1( "CDocText::UpdateInternalLines() %dms\n", GetTickCount()-dwTicksStart );
}

void CDocText::RemoveInternalLines( POSLINE posStart, int nCount )
{
	if ( nCount < 1 )
		return;
	ASSERT(POSLINE_ISVALID(posStart));
	SetModifiedFlag();
	POSLINE pos = posStart;	// position to remove
	POSLINE posN = posStart;
	for ( int n = 0; n < nCount; ++n )
	{
		ASSERT( POSLINE_ISVALID(pos) );
		if ( (DWORD)pos <= 2 )
			return;	// error
		const LineData &ld = GetLineDataAt( posN );
		GetNextLine( posN );
		CMapElem el;
		VERIFY( m_mapHashToLines.Lookup( ld.dwHash, el ) );
		el.Remove( pos );
		m_mapHashToLines[ ld.dwHash ] = el;
		el.Free();
		if ( el.GetLineData() != NULL ) {
			LineData &ldn = GetLineDataAt( el.GetLineData() );
			ldn.bUnique = TRUE;
		}
		m_listLines.RemoveAt( (POSITION)pos );
		pos = posN;
	}
	Renumber( pos );
}

void CDocText::Renumber( POSLINE posStart )
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	int nLine;
	if ( !POSLINE_ISVALID(posStart) )
	{
		posStart = GetFirstLine();
		nLine = 0;
	}
	else
	{
		POSLINE pos = posStart;
		GetPrevLine( pos );
		nLine = ( pos == POSLINE_TOP ?	0 : GetLineDataAt( pos ).nLine );
	}
	while ( POSLINE_ISVALID(posStart) )
	{
		m_listLines.GetAt( (POSITION)posStart ).nLine = ++nLine;
		GetNextLine( posStart );
	}
	TRACE1( "CDocText::Renumber() %dms\n", GetTickCount()-dwTicksStart );
}

BOOL CDocText::IsNotEmpty(const CString & str)
{
	for ( int n = 0; n < str.GetLength(); ++n )
	{
		if ( str[n] != ' ' && str[n] != '\t' )
			return TRUE;
	}
	return FALSE;
}

DWORD CDocText::GetHash(const CString & str)
{
	if ( str.GetLength() < 5 )		// 161021: avoid handling short lines as unique
		return -1;

	DWORD dwCRC16 = 0;
	for ( int n = 0; n < str.GetLength(); ++n )
	{
		DWORD v = dwCRC16 ^ str[n];
		for ( int i = 0; i < 8; ++i )
		{
			if ( (v & 1) != 0 )
				v =(v/2) ^0xA001;
			else
				v /= 2;
		}
		dwCRC16 = v;
	}
	return dwCRC16;
}

BOOL CDocText::GetNextUniqueLine( POSLINE &pos, const POSLINE posEnd ) const 
{ 
	ASSERT(POSLINE_ISVALID(pos));
	GetNextLine( pos );
	while ( POSLINE_ISVALID(pos) && pos != posEnd )
	{
		if ( GetLineDataAt( pos ).bUnique )
			return TRUE;
		GetNextLine( pos );
	}
	return FALSE;
}

BOOL CDocText::GetPrevUniqueLine( POSLINE &pos, const POSLINE posEnd ) const 
{ 
	ASSERT(POSLINE_ISVALID(pos));
	GetPrevLine( pos );
	while ( POSLINE_ISVALID(pos) && pos != posEnd )
	{
		if ( GetLineDataAt( pos ).bUnique )
			return TRUE;
		GetPrevLine( pos );
	}
	return FALSE;
}

int CDocText::GetLineCount(POSLINE posStart, POSLINE posEnd) const 
{
	if ( !POSLINE_ISVALID(posStart) || posEnd == POSLINE_TOP )
		return 0;
	int nStart = 0;
	if ( POSLINE_ISVALID(posStart) )
		nStart = GetLineDataAt( posStart ).nLine;
	if ( posEnd == NULL || posEnd == POSLINE_END )
		return m_listLines.GetTail().nLine + 1 - nStart; 
//	TRACE2( "GetLineCount %d - %d ", m_listLines.GetAt( posEnd ).nLine, m_listLines.GetAt( posStart ).nLine );
	return GetLineDataAt( posEnd ).nLine - nStart; 
}

BOOL CDocText::FindMatch( POSLINE &pos, const POSLINE posU, 
						  const LineData &ld ) const
{
	while ( POSLINE_ISVALID(pos) && pos != posU )
	{
		if ( CompareLine( pos, ld ) )
		{
			return TRUE;
		}
		GetNextLine( pos );
	}
	return FALSE;
}

BOOL CDocText::FindMatch( POSLINE &pos, const POSLINE posU, 
						  const CDocText *pDocOther, POSLINE &posOther, const POSLINE posOtherU ) const
{
	POSLINE posS = pos;		// save start position
	POSLINE posOtherS = posOther;
	POSLINE posT = pos;		// top position is pair wise incremented
	POSLINE posOtherT = posOther;
	GetNextLine( posT );
	pDocOther->GetNextLine( posOtherT );

	while ( POSLINE_ISVALID(posT) && posT != posU && POSLINE_ISVALID(posOtherT) && posOtherT != posOtherU )
	{
		const LineData &ldT = GetLineDataAt( posT );
		const LineData &ldOtherT = pDocOther->GetLineDataAt( posOtherT );
		if ( CompareLine( ldT, ldOtherT ) )
		{
			pos = posT;
			posOther = posOtherT;
			return TRUE;
		}
		pos = posS;		// restore start position
		posOther = posOtherS;
		while ( pos != posT && posOther != posOtherT )
		{
			if ( CompareLine( pos, ldOtherT ) )
			{
				posOther = posOtherT;
				return TRUE;
			}
			if ( pDocOther->CompareLine( posOther, ldT ) )
			{
				pos = posT;
				return TRUE;
			}
			GetNextLine( pos );
			pDocOther->GetNextLine( posOther );
		}
		GetNextLine( posT );
		pDocOther->GetNextLine( posOtherT );
	}
	while ( POSLINE_ISVALID(posT) && posT != posU && (!POSLINE_ISVALID(posOtherT) || posOtherT == posOtherU) )
	{
		POSLINE posOther = posOtherS;	// restore start position
		const LineData &ld = GetLineDataAt( posT );
		if ( pDocOther->FindMatch( posOther, posOtherT, ld ) )
		{
			pos = posT;
			return TRUE;
		}
		GetNextLine( posT );
	}
	while ( (!POSLINE_ISVALID(posT) || posT == posU) && POSLINE_ISVALID(posOtherT) && posOtherT != posOtherU )
	{
		POSLINE pos = posS;	// restore start position
		const LineData &ldOther = pDocOther->GetLineDataAt( posOtherT );
		if ( FindMatch( pos, posT, ldOther ) )
		{
			posOther = posOtherT;
			return TRUE;
		}
		pDocOther->GetNextLine( posOtherT );
	}
	return FALSE;
}

BOOL CDocText::FindMatchUnique1( POSLINE &pos, const POSLINE posU, 
						  const LineData &ld ) const
{
	if ( !ld.bUnique )
		return FALSE;
	while ( POSLINE_ISVALID(pos) && pos != posU )
	{
		const LineData &ldOwn = GetLineDataAt( pos );
		if ( ldOwn.bUnique )
		{
			if ( CompareLine( ldOwn, ld ) )
			{
				return TRUE;
			}
		}
		GetNextLine( pos );
	}
	return FALSE;
}

BOOL CDocText::FindMatchUnique1 ( POSLINE &pos, const POSLINE posU, 
						  const CDocText *pDocOther, POSLINE &posOther, const POSLINE posOtherU ) const
{
	ASSERT( POSLINE_ISVALID(pos) );
	if ( !GetLineDataAt( pos ).bUnique )
		GetNextUniqueLine( pos, posU );

	ASSERT( POSLINE_ISVALID(posOther) );
	if ( !pDocOther->GetLineDataAt( posOther ).bUnique )
		pDocOther->GetNextUniqueLine( posOther, posOtherU );

	POSLINE posS = pos;		// save start position
	POSLINE posOtherS = posOther;
	POSLINE posT = pos;		// top position is pair wise incremented
	POSLINE posOtherT = posOther;
	while ( POSLINE_ISVALID(posT) && posT != posU && POSLINE_ISVALID(posOtherT) && posOtherT != posOtherU )
	{
		const LineData &ldT = GetLineDataAt( posT );
		const LineData &ldOtherT = pDocOther->GetLineDataAt( posOtherT );
		ASSERT ( ldT.bUnique && ldOtherT.bUnique );
		if ( CompareLine( ldT, ldOtherT ) )
		{
			pos = posT;
			posOther = posOtherT;
			return TRUE;
		}
		pos = posS;		// restore start position
		posOther = posOtherS;
		while ( pos != posT && posOther != posOtherT )
		{
			const LineData &ldOwn = GetLineDataAt( pos );
			if ( CompareLine( ldOwn, ldOtherT ) )
			{
				posOther = posOtherT;
				return TRUE;
			}
			const LineData &ldOther = pDocOther->GetLineDataAt( posOther );
			if ( pDocOther->CompareLine( ldOther, ldT ) )
			{
				pos = posT;
				return TRUE;
			}
			if ( !GetNextUniqueLine( pos, posT ) )
				break;
			if ( !pDocOther->GetNextUniqueLine( posOther, posOtherT ) )
				break;
		}
		GetNextUniqueLine( posT, posU );
		pDocOther->GetNextUniqueLine( posOtherT, posOtherU );
	}
	while ( POSLINE_ISVALID(posT) && posT != posU && (!POSLINE_ISVALID(posOtherT) || posOtherT == posOtherU) )
	{
		posOther = posOtherS;	// restore start position
		const LineData &ld = GetLineDataAt( posT );
		if ( pDocOther->FindMatchUnique1( posOther, posOtherT, ld ) )
		{
			pos = posT;
			return TRUE;
		}
		if ( !GetNextUniqueLine( posT, posU ) )
			return FALSE;
	}
	while ( (!POSLINE_ISVALID(posT) || posT == posU) && POSLINE_ISVALID(posOtherT) && posOtherT != posOtherU )
	{
		pos = posS;	// restore start position
		const LineData &ldOther = pDocOther->GetLineDataAt( posOtherT );
		if ( FindMatchUnique1( pos, posT, ldOther ) )
		{
			posOther = posOtherT;
			return TRUE;
		}
		if ( !pDocOther->GetNextUniqueLine( posOtherT, posOtherU ) )
			return FALSE;
	}
	return FALSE;
}

POSLINE CDocText::FindLineNo( int nLineNo, POSLINE posHint )
{
	if ( nLineNo < 1 )
		return POSLINE_TOP;

	int nLineNoHint = 0;
	if ( POSLINE_ISVALID(posHint) )
		nLineNoHint = GetLineNo( posHint );
	int nLineNoMax = (int)m_listLines.GetCount();
	if ( nLineNo > nLineNoMax )
		return POSLINE_END;

	BOOL bBackward = FALSE;
	POSLINE pos;
	if ( nLineNo < nLineNoHint )
	{
		if ( nLineNo < (nLineNoHint/2) )
		{
			pos = GetFirstLine();
		}
		else
		{
			bBackward = TRUE;
			pos = posHint;
		}
	}
	else	// nLineNo >= nLineNoHint
	{
		if ( nLineNo < ((nLineNoHint+nLineNoMax) / 2) )
		{
			pos = ( POSLINE_ISVALID(posHint) ? posHint : GetFirstLine() );
		}
		else
		{
			bBackward = TRUE;
			pos = GetLastLine();
		}
	}
	if ( bBackward )
	{
		while ( POSLINE_ISVALID(pos) && nLineNo != GetLineNo( pos ) )
			GetPrevLine( pos );
	}
	else
	{
		while ( POSLINE_ISVALID(pos) && nLineNo != GetLineNo( pos ) )
			GetNextLine( pos );
	}
	ASSERT( POSLINE_ISVALID(pos) ? nLineNo == GetLineNo(pos) : TRUE );
	return pos;
}

void CDocText::GetUniqueLines( POSLINE pos, const POSLINE posEnd, CPosArray &apos ) const
{
	apos.SetSize( 0, 512 );
	CMap<DWORD,DWORD,int,int> mapHash;

	while ( POSLINE_ISVALID(pos) && pos != posEnd )
	{
		BOOL bUnique = TRUE;
		const LineData &ld = GetLineDataAt( pos );

		if ( ld.dwHash == 0 )	// skip empty lines
		{
			GetNextLine( pos );
			continue;
		}
		int nHash;
		if ( mapHash.Lookup( ld.dwHash, nHash ) )
		{
			if ( nHash >= 0 )
			{
				apos[nHash] = NULL;		// mark for delete
				mapHash[ld.dwHash] = -1;
			}
		}
		else
		{
			mapHash[ ld.dwHash ] = (int)apos.Add( pos );
		}

		GetNextLine( pos );
	}

	for ( int n = (int)apos.GetSize()-1; n >= 0; --n )
	{
		if ( apos[n] == NULL )
			apos.RemoveAt(n);
	}
}

void CDocText::UpdateStringDiff(const POSLINE posOwn, CDocText *pOther, const POSLINE posOther)
{
	if ( POSLINE_ISVALID(posOwn) )
	{
		LineData &ld = GetLineDataAt( posOwn );
		if ( !POSLINE_ISVALID(posOther) )
			ld.strDiff.ResetAll();
		else
		{
			LineData &ldOther = pOther->GetLineDataAt( posOther );
			ld.strDiff.Compare( ld.strExp, ldOther.strExp, ldOther.strDiff );
		}
	}
}

void CDocText::CombineWordsForLines( CMapStringToPtr &map, POSLINE pos, const POSLINE posEnd )
{
#ifdef _DEBUG
	DWORD dwTicksStart = GetTickCount();
#endif
	map.RemoveAll();
	DWORD dwMode = 0xFFFF;
	while ( POSLINE_ISVALID(pos) && pos != posEnd )	// iterate over the given line range
	{
		if ( map.GetCount() > 1000 )
			dwMode = 4;
		LineData &ld = GetLineDataAt( pos );
		ld.posOther = NULL;
		CStringList lstrCW;
		ld.GetCombinedWords( lstrCW, dwMode );
		POSITION posCW = lstrCW.GetHeadPosition();
		while ( posCW != NULL )
		{
			const CString strCW = lstrCW.GetNext( posCW );
			void *posTarget;
			if ( map.Lookup( strCW, posTarget ) )
				map[ strCW ] = NULL;	// no longer unique
			else if (map.GetCount() < 10000)
				map[ strCW ] = pos;		// unique LineData pos
		}
		GetNextLine( pos );
	}
	TRACE2( "CDocText::CombineWordsForLines() n=%d %dms\n", map.GetCount(), GetTickCount()-dwTicksStart );
}

void CDocText::MarkLinesFromWordMaps( CMapStringToPtr &map, CDocText *pDocOther, CMapStringToPtr &mapOther )
{
	POSITION pos = map.GetStartPosition();
	while ( pos != NULL )	// iterate over map
	{
		CString strCombWords;
		POSLINE posLineData;
		map.GetNextAssoc( pos, strCombWords, (void*&)posLineData );
		if ( POSLINE_ISVALID(posLineData) )	// for unique entry
		{
			POSLINE posLineDataOther;
			if ( mapOther.Lookup( strCombWords, (void*&)posLineDataOther ) )
			{
				if ( POSLINE_ISVALID(posLineDataOther) )		// both unique
				{
//					TRACE1( "CDocText::MarkLinesFromWordMaps() %s\n", strCombWords );
					LineData &ld = GetLineDataAt( posLineData );
					ld.posOther = posLineDataOther;
					LineData &ldOther = pDocOther->GetLineDataAt( posLineDataOther );
					ldOther.posOther = posLineData;
				}
			}
		}
	}
}

void CDocText::GetUniqueWordsLines( POSLINE pos, const POSLINE posEnd, CPosArray &apos )
{
	apos.SetSize( 0, 512 );
	while ( POSLINE_ISVALID(pos) && pos != posEnd )	// iterate over line range
	{
		const LineData &ld = GetLineDataAt( pos );
		if ( ld.posOther != NULL )
			apos.Add( pos );
		GetNextLine( pos );
	}
}

POSLINE CDocText::FindMatchUniqueWords( CPosArray &apos, int offs,
									const POSLINE posOther ) const
{
	for ( int n = offs; n < apos.GetSize(); ++n )
	{
		if ( CompareWordsLine( apos[n], posOther ) )
		{
			return apos[n];
		}
	}
	return NULL;
}

BOOL CDocText::FindMatchUniqueWords ( POSLINE &pos, CPosArray &apos, int &ni, 
					const CDocText *pDocOther, POSLINE &posOther, CPosArray &aposOther, int &oi ) const
{
	int nMaxIdx = (int)apos.GetSize();
	int nLine = GetLineDataAt(pos).nLine;
	for ( ; ni < nMaxIdx; ++ni )
	{
		if ( GetLineDataAt(apos[ni]).nLine >= nLine )
			break;
	}
	int nMaxIdxOther = (int)aposOther.GetSize();
	nLine = pDocOther->GetLineDataAt(posOther).nLine;
	for ( ; oi < nMaxIdxOther; ++oi )
	{
		if ( pDocOther->GetLineDataAt(aposOther[oi]).nLine >= nLine )
			break;
	}

	int t;
	for ( t = 0; (ni + t) < nMaxIdx && (oi + t) < nMaxIdxOther; ++t )
	{
//		const LineData &ldT = m_listLines.GetAt( apos[ni+t] );
//		const LineData &ldOtherT = pDocOther->m_listLines.GetAt( aposOther[oi+t] );
		POSLINE posT = apos[ni+t];
		POSLINE posOtherT = aposOther[oi+t];
		if ( CompareWordsLine( posT, posOtherT ) )
		{
			pos = posT;
			posOther = posOtherT;
			return TRUE;
		}
		for ( int i = 0;  i < t; ++i )
		{
//			const LineData &ldOwn = m_listLines.GetAt( apos[ni+i] );
			POSLINE posI = apos[ni+i];
			if ( CompareWordsLine( posI, posOtherT ) )
			{
				pos = posI;
				posOther = posOtherT;
				return TRUE;
			}
//			const LineData &ldOther = pDocOther->m_listLines.GetAt( aposOther[oi+i] );
			POSLINE posOtherI = aposOther[oi+i];
			if ( pDocOther->CompareWordsLine( posOtherI, posT ) )
			{
				pos = posT;
				posOther = posOtherI;
				return TRUE;
			}
		}
	}
	for ( ; (ni + t) < nMaxIdx && (oi + t) >= nMaxIdxOther; ++t )
	{
//		const LineData &ld = m_listLines.GetAt( apos[ni+t] );
		posOther = pDocOther->FindMatchUniqueWords( aposOther, oi, apos[ni+t] );
		if ( POSLINE_ISVALID(posOther) )
		{
			pos = apos[ni+t];
			return TRUE;
		}
	}
	for ( ; (ni + t) >= nMaxIdx && (oi + t) < nMaxIdxOther; ++t )
	{
//		const LineData &ldOther = pDocOther->m_listLines.GetAt( aposOther[oi+t] );
		pos = FindMatchUniqueWords( apos, ni, aposOther[oi+t] );
		if ( POSLINE_ISVALID(pos) )
		{
			posOther = aposOther[oi+t];
			return TRUE;
		}
	}
	return FALSE;
}

