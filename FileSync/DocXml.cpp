#include "StdAfx.h"
#include "DocXml.h"

CDocXml::CDocXml(void)
{
}

CDocXml::~CDocXml(void)
{
}

// CDocFileSync serialization

void CDocXml::DeleteContents()
{
	CDocText::DeleteContents();
	m_astrTags.RemoveAll();
	m_nTagLevel = 0;
	m_bInTag = FALSE;
	m_bInComment = FALSE;
	m_bInString = FALSE;
}

IMPLEMENT_DYNCREATE(CDocXml, CDocText)

void CDocXml::StoreLine( int &nLine, int &nLinesCR, CString strLine )
{
	BOOL bCR = FALSE;
	if ( strLine.Right(1) == _T("\r") )
	{
		bCR = TRUE;
		int nLen = strLine.GetLength();
		TCHAR *pLine = strLine.GetBuffer(nLen);
		strLine.ReleaseBuffer( nLen - 1 );
	}

	while ( strLine.GetLength() > 0 )
	{
		DelWhiteSpace( strLine );
		int nLen = strLine.GetLength();
		if (m_bInComment)
		{
			int nEnd = strLine.Find( _T("-->") );
			if ( nEnd >= 0 )
			{
				m_bInComment = FALSE;
				if ( nEnd > 0 )
					StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd ) );
				StoreSubLine( nLine, bCR, nLinesCR, _T("-->") );
				strLine = strLine.Mid( nEnd+3 );
				continue;
			}
			StoreSubLine( nLine, bCR, nLinesCR, strLine );
			break;
		}
		int nComment = strLine.Find( _T("<!--") );
		if ( nComment > 0 )
			StoreLine( nLine, nLinesCR, strLine.Left( nComment ) ); // recurse to store data in front of comment
		if ( nComment >= 0 )
		{
			m_bInComment = TRUE;
			StoreSubLine( nLine, bCR, nLinesCR, _T("<!--") );
			strLine = strLine.Mid( nComment+4 );
			continue;
		}

		BOOL bStartTag = FALSE;
		if ( strLine[0] == '<' && strLine[1] != '/' && strLine[1] != '?' && strLine[1] != '!' )
		{
			// start of new tag
			// find end of tag name: > WS /
			int nEnd = strLine.FindOneOf( _T("> \t/") );
			if ( nEnd < 0 )	// EOL
				nEnd = nLen;
//			TRACE2( "Tag store %d %s \n", m_nTagLevel, strLine.Mid( 1, nEnd-1 ) );
			m_astrTags.SetSize(m_nTagLevel+1);
			m_astrTags.SetAt(m_nTagLevel, strLine.Mid( 1, nEnd-1 ) );
			if ( nEnd == nLen || strLine[nEnd] == ' ' || strLine[nEnd] == '\t' )
			{
				// attr follows
				StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd ) );
				strLine = strLine.Mid( nEnd );
				m_bInTag = TRUE;
				++m_nTagLevel;
				continue;
			}
			// no attr
//			++m_nTagLevel;
			bStartTag = TRUE;
		}
		if ( m_bInTag || bStartTag )
		{
			// find attr: >= (end of tag or attr=val)
			int nEnd = strLine.FindOneOf( _T(">=") );
			if ( nEnd >= 0 && strLine[nEnd] == '=' )
			{
				nEnd = SkipWhiteSpace( strLine, nEnd+1 );
				if ( strLine[nEnd] == '"' )	// match quotes
				{
					nEnd = 1 + strLine.Find( '"', nEnd+1 );
					if ( nEnd < 1 )
						nEnd = nLen;
					nEnd = SkipWhiteSpace( strLine, nEnd );
					if ( nEnd < (nLen-1) && strLine[nEnd] == '/' )
						++nEnd;		// -> ">"
				}
				else
				{	// bad syntax: find end of value
					int nEnd2 = strLine.Find( ' ', nEnd );
					int nEnd3 = strLine.Find( '\t', nEnd );
					int nEnd4 = strLine.Find( '>', nEnd );
					nEnd = nLen;
					if ( nEnd2 >= 0 )
						nEnd = nEnd2;
					if ( nEnd3 >= 0 && nEnd3 < nEnd)
						nEnd = nEnd3;
					if ( nEnd4 >= 0 && nEnd4 < nEnd)
						nEnd = nEnd4;
				}
			}
			if ( nEnd >= 0 && strLine[nEnd] == '>' )
			{
				if ( nEnd > 0 && strLine[nEnd-1] == '/' )	// empty tag finished
				{
//					if ( bStartTag )
//						--m_nTagLevel;
					StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd+1 ) );
					strLine = strLine.Mid( nEnd+1 );
					if ( !bStartTag )
						--m_nTagLevel;
					m_bInTag = FALSE;
					continue;
				}
				// check for inline tagged chardata
				int nEnd2 = SkipWhiteSpace( strLine, nEnd+1 );
				nEnd2 = strLine.Find( '<', nEnd2 );
				if ( nEnd2 >= 0 && nEnd2 < (nLen-1) && strLine[nEnd2+1] == '/' )
				{
					int nEnd3 = strLine.Find( '>', nEnd2 );
					if ( nEnd3 >= 0 )
					{
						StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd3+1 ) );
						if ( !bStartTag )
							--m_nTagLevel;
//						TRACE3( "Tag pair match %d %s#%s \n", m_nTagLevel, m_astrTags[m_nTagLevel], strLine.Mid( nEnd2+2, nEnd3-nEnd2-2 ) );
						ASSERT( m_astrTags[m_nTagLevel] == strLine.Mid( nEnd2+2, nEnd3-nEnd2-2 ) );
						strLine = strLine.Mid( nEnd3+1 );
						m_bInTag = FALSE;
						continue;
					}
				}
				// finish tag
				StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd+1 ) );
				strLine = strLine.Mid( nEnd+1 );
				m_bInTag = FALSE;
				if ( bStartTag )
					++m_nTagLevel;
				continue;
			}
			// store attr
			if ( nEnd < 0 )
				nEnd = nLen;
			StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd ) );
			strLine = strLine.Mid( nEnd );
			continue;
		}

		if ( strLine[0] == '<' && strLine[1] == '/' && strLine[1] != '?' && strLine[1] != '!' )
		{
			// start of end tag
			// find end of tag name: >
			int nEnd = 1 + strLine.Find( '>' );
			if ( nEnd < 1 )	// EOL
				nEnd = nLen;
			if ( m_nTagLevel > 0 )
				--m_nTagLevel;
//			TRACE3( "Tag match %d %s#%s \n", m_nTagLevel, m_astrTags[m_nTagLevel], strLine.Mid( 2, nEnd-3 ) );
			ASSERT( m_astrTags[m_nTagLevel] == strLine.Mid( 2, nEnd-3 ) );
			StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd ) );
			strLine = strLine.Mid( nEnd );
			m_bInTag = FALSE;
			continue;
		}
		// data
		// find end of data
		int nEnd = strLine.Find( '<' );
		if ( nEnd < 1 )	// too short or EOL
			nEnd = nLen;
		StoreSubLine( nLine, bCR, nLinesCR, strLine.Left( nEnd ) );
		strLine = strLine.Mid( nEnd );
	}
}

void CDocXml::StoreSubLine( int &nLine, BOOL bCR, int &nLinesCR, const CString strLine )
{
	int nFill = 2*m_nTagLevel;
	if ( m_bInTag )
		nFill += 2;
	CString strFill( ' ', nFill );
	LineData ld;
	ld.strRaw = strFill + strLine;
	ld.nLine = ++nLine;
	if ( bCR )
		++nLinesCR;
	m_listLines.AddTail( ld );
}

void CDocXml::DelWhiteSpace( CString &strLine )
{
	int nLen = strLine.GetLength();
	int nStart = 0;
	TCHAR c = strLine[nStart];
	while( (nStart < nLen) && (c == ' ' || c == '\t') )
	{
		c = strLine[++nStart];
	}
	strLine = strLine.Mid( nStart );
}

int CDocXml::SkipWhiteSpace( const CString &strLine, int nStart )
{
	int nLen = strLine.GetLength();
	TCHAR c = strLine[nStart];
	while( (nStart < nLen) && (c == ' ' || c == '\t') )
	{
		c = strLine[++nStart];
	}
	return nStart;
}

