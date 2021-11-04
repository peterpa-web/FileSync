#pragma once
#include "DocText.h"

class CDocXml :
	public CDocText
{
	DECLARE_DYNCREATE(CDocXml)

public:
	CDocXml(void);
	virtual ~CDocXml(void);

	virtual void DeleteContents();
	virtual void StoreLine( int &nLine, int &nLinesCR, CString strLine );

protected:
//	void RestoreFile( CFile *pFile );
	void StoreSubLine( int &nLine, BOOL bCR, int &nLinesCR, const CString strLine );
	void DelWhiteSpace( CString &strLine );
	int SkipWhiteSpace( const CString &strLine, int nStart );
	CStringArray m_astrTags;
	int m_nTagLevel;
	BOOL m_bInTag;
	BOOL m_bInComment;
	BOOL m_bInString;
};
