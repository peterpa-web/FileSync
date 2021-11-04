#pragma once
#include "DirEntry.h"

class CDirEntry;
class CDocDir;

typedef CList<CDirEntry> CListDirEntriesBase;

class CListDirEntries: public CListDirEntriesBase
{
public:
	CListDirEntries() : CListDirEntriesBase( 30 ) { m_pos = NULL; }
	void RemoveAll();
	void ResetAll();
	void RemoveAt( POSITION pos );
	DOCPOS Insert( const CDirEntry &de );
	DOCPOS Find( BOOL bDir, const CString &strName ) const;
	DOCPOS Find( const CDirEntry &de ) const;
	DOCPOS Find( const CDocDir *pDir ) const;
	void SetFlags( DWORD maskCond, DWORD maskRes, DWORD maskSet );
	void Copy(CListDirEntries* pSrc);
//    CListDirEntries & operator =(const CListDirEntries &src) { ASSERT(FALSE); return *this; }
#ifdef _DEBUG
	void AssertValid() const;
#endif

protected:
	POSITION m_pos;
};

