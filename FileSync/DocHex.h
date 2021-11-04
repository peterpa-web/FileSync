#pragma once
#include "afxtempl.h"
#include "DocFile.h"

class CDocHex :
	public CDocFile
{
	DECLARE_DYNCREATE(CDocHex)

public:
	CDocHex(void);
	virtual ~CDocHex(void);

	virtual void DeleteContents();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual int GetMaxLineLen() const { return 66; }

	CString GetLineNo( int offs ) const;
	CString GetDisplayLine( int offs, int len ) const;
	BOOL SearchLine( const CString& strSearch, int offs, int len ) const;
	void FindMatchUnique ( int &mn, int &ni,
						  CDocHex *pOther, int &mo, int &oi ) const;
	int GetDataSize() { return (int)m_aData.GetSize(); }
	BOOL IsEqual( int n, const CDocHex *pOther, int o ) { return m_aData[n] == pOther->m_aData[o]; }

protected:
	CByteArray m_aData;
	CDWordArray m_adwIdx;	// unique positions

	void StoreFile( CFile *pFile );
	void RestoreFile( CFile *pFile );
	void CreateUniqueIdx();
};

