#pragma once
#include "StringDiff.h"
#include "DocFile.h"

#include "afxtempl.h"

struct __POSLINE { DWORD d; };
typedef struct __POSLINE *POSLINE;
#define POSLINE_TOP ((POSLINE)1)
#define POSLINE_END ((POSLINE)2)
#define POSLINE_ISVALID(pos) ((DWORD)pos > 2)

typedef CTypedPtrArray<CPtrArray,POSLINE> CPosArray;

class CDocText :
	public CDocFile
{
	DECLARE_DYNCREATE(CDocText)

public:
	static LPCTSTR s_pszEncodingDefault;
	static LPCTSTR s_pszEncodingUnicode;
	static LPCTSTR s_pszEncodingUnicodeRev;
	static LPCTSTR s_pszEncodingUTF8;
	static LPCTSTR s_pszEncodingXUTF8;

	CDocText(void);
	virtual ~CDocText(void);

	class LineData
	{
	public:
		LineData();
		~LineData() {}

		CString strRaw;
		CString strExp;
		CStringDiff strDiff;
		DWORD dwHash;
		int nLine;
		BOOL bUnique;
//		POSITION posSameHash;
		POSLINE posOther;		// MatchWords

		void GetWords( CStringList &lstr ) const;		// MatchWords
		void GetCombinedWords( CStringList &lstr, DWORD dwMode = 0xFFFF ) const;
	};

	typedef CTypedPtrList<CPtrList,POSLINE> CPosList;

	class CMapElem
	{
	public:
		CMapElem();
		CMapElem( POSLINE pos );
		~CMapElem();
		POSLINE GetFirstLineData();
		POSLINE GetLineData() { return m_posLineData; }
		void Free() { m_pPosList = NULL; }		// free temporay copy to avoid deletion
		void Add( POSLINE pos );
		void Remove (POSLINE pos );

	protected:
		POSLINE m_posLineData;
		CPosList *m_pPosList;
	};

	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual void StoreLine( int &nLine, int &nLinesCR, CString strLine );

	const CString GetDisplayLine( POSLINE pos ) const { return ( POSLINE_ISVALID(pos) ? GetLineDataAt(pos).strExp : m_strEmpty ); }
	int GetDisplayLineLen( POSLINE pos ) const { return GetDisplayLine(pos).GetLength(); }
	CStringDiff GetStringDiff( POSLINE pos ) const { return ( POSLINE_ISVALID(pos) ? GetLineDataAt(pos).strDiff : CStringDiff() ); }
	const CString GetRawLine( POSLINE pos ) const { return ( POSLINE_ISVALID(pos) ? GetLineDataAt(pos).strRaw : m_strEmpty ); }
	int GetLineNo( POSLINE pos ) const { return ( POSLINE_ISVALID(pos) ? GetLineDataAt(pos).nLine : (pos == POSLINE_END ? INT_MAX: 0) ); }
	BOOL IsUnique( POSLINE pos ) const { return ( POSLINE_ISVALID(pos) ? GetLineDataAt(pos).bUnique : FALSE ); }
	virtual int GetMaxLineLen() const { return m_nMaxLineLen; }
	int GetTabSize() const { return m_nTabExpand; }
	void SetTabSize( int n ) { m_nTabExpand = n; }		// need UpdateInternalLines
	BOOL IsCompactSpace() const { return m_bCompactSpace; }
	void SetCompactSpace( BOOL b ) { m_bCompactSpace = b; }		// need UpdateInternalLines
	BOOL IsUnixFormat() const { return m_bUnixFormat; }
	void SetUnixFormat( BOOL b ) { m_bUnixFormat = b; }
	CString GetEncoding() const { return m_strEncoding; }
	void SetEncoding( const CString &str ) { m_strEncoding = str; }

	POSLINE GetFirstLine() const { return (POSLINE)m_listLines.GetHeadPosition(); }
	POSLINE GetLastLine() const { return (POSLINE)m_listLines.GetTailPosition(); }
	void GetNextLine( POSLINE &pos ) const { m_listLines.GetNext( (POSITION&)pos ); if (pos==NULL) pos=POSLINE_END; }
	void GetPrevLine( POSLINE &pos ) const { m_listLines.GetPrev( (POSITION&)pos ); if (pos==NULL) pos=POSLINE_TOP; }
	const LineData& GetLineDataAt( const POSLINE pos ) const { return m_listLines.GetAt( (const POSITION)pos ); }
	LineData& GetLineDataAt( const POSLINE pos ) { return m_listLines.GetAt( (const POSITION)pos ); }
	void GetPrevLine( POSLINE &pos, int n ) const { while ( --n >= 0 ) m_listLines.GetPrev( (POSITION&)pos ); }
	BOOL GetNextUniqueLine( POSLINE &pos, const POSLINE posEnd ) const;
	BOOL GetPrevUniqueLine( POSLINE &pos, const POSLINE posEnd ) const;
	void GetUniqueLines( POSLINE pos, const POSLINE posEnd, CPosArray &apos ) const;
	int GetLineCount(POSLINE posStart, POSLINE posEnd) const;
	int GetLineCount() const { return (int)m_listLines.GetCount(); }
	POSLINE InsertInternalLines( POSLINE &posStart, const CStringArray &astrLines );
	void RemoveInternalLines( POSLINE posStart, int nCount );
	void UpdateInternalLines();
	POSLINE FindLineNo( int nLineNo, POSLINE posHint );
	BOOL CompareLine(const LineData &ldOwn, const LineData &ldOther) const;
	BOOL CompareLine(const POSLINE posOwn, const LineData &ldOther) const;
	BOOL CompareLine(const POSLINE posOwn, const CDocText *pOther, const POSLINE posOther) const;
	BOOL FindMatch( POSLINE &pos, const POSLINE posU, const LineData &ld ) const;
	BOOL FindMatch( POSLINE &pos, const POSLINE posU, 
					const CDocText *pDocOther, POSLINE &posOther, const POSLINE posOtherU ) const;
	BOOL FindMatchUnique1( POSLINE &pos, const POSLINE posU, const LineData &ld ) const;
	BOOL FindMatchUnique1( POSLINE &pos, const POSLINE posU, 
					const CDocText *pDocOther, POSLINE &posOther, const POSLINE posOtherU ) const;
//	POSITION FindMatchUnique2( CPosArray &apos, int offs, const LineData &ld ) const;
//	BOOL FindMatchUnique2( POSITION &pos, CPosArray &apos, int &ni, 
//					const CDocText *pDocOther, POSITION &posOther, CPosArray &aposOther, int &oi ) const;
	void UpdateStringDiff(const POSLINE posOwn, CDocText *pOther, const POSLINE posOther);
		// MatchWords
	void CombineWordsForLines( CMapStringToPtr &map, POSLINE pos, const POSLINE posEnd );	// MatchWords
	void MarkLinesFromWordMaps( CMapStringToPtr &map, CDocText *pDocOther, CMapStringToPtr &mapOther );
	void GetUniqueWordsLines( POSLINE pos, const POSLINE posEnd, CPosArray &apos );
	BOOL CompareWordsLine(const POSLINE posOwn, const POSLINE posOther) const;
	POSLINE FindMatchUniqueWords( CPosArray &apos, int offs, const POSLINE posOther ) const;
	BOOL FindMatchUniqueWords( POSLINE &pos, CPosArray &apos, int &ni, 
					const CDocText *pDocOther, POSLINE &posOther, CPosArray &aposOther, int &oi ) const;

protected:
	CList<LineData,LineData&> m_listLines;
	CMap<DWORD,DWORD,CMapElem,CMapElem> m_mapHashToLines;
	int m_nMaxLineLen;
	int m_nTabExpand;
	BOOL m_bCompactSpace;
	BOOL m_bUnixFormat;
	CString m_strEncoding;
	CString m_strEmpty;

	void StoreFile( CFile *pFile );
	void RestoreFile( CFile *pFile );
	POSLINE InsertInternalLine( POSLINE pos, const CString &strLine );	// insert before pos
	void UpdateInternalLine( POSLINE pos );
	static BOOL IsNotEmpty(const CString & str);
	static DWORD GetHash(const CString & str);
	void Renumber( POSLINE posStart );
};

inline BOOL CDocText::CompareLine(const LineData &ldOwn, const LineData &ldOther) const
{
	if ( ldOwn.dwHash != ldOther.dwHash )
		return FALSE;
	if ( ldOwn.strExp != ldOther.strExp )
		return FALSE;
	return TRUE;
}

inline BOOL CDocText::CompareLine(const POSLINE posOwn, const LineData &ldOther) const
{
	const LineData &ldOwn = GetLineDataAt( posOwn );
	return CompareLine( ldOwn, ldOther );
}

inline BOOL CDocText::CompareLine(const POSLINE posOwn, const CDocText *pOther, const POSLINE posOther) const
{
	const LineData &ldOwn = GetLineDataAt( posOwn );
	const LineData &ldOther = pOther->GetLineDataAt( posOther );
	return CompareLine( ldOwn, ldOther );
}

inline BOOL CDocText::CompareWordsLine(const POSLINE posOwn, const POSLINE posOther) const
{
	const LineData &ldOwn = GetLineDataAt( posOwn );
	return ( ldOwn.posOther == posOther );
}

