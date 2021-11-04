#pragma once

class CStringDiff
{
public:
	CStringDiff(void);
	~CStringDiff(void);

	void ResetAll();
	void Compare( const CString &strOwn, const CString &strOther, CStringDiff &strDiffOther );
	BOOL operator [] ( int n ) const { return ( n >= m_strMark.GetLength() ? TRUE : m_strMark[n] ); }
	CString Merge( const CString &strOwn, int nPosOwn, const CString &strOther ); 
		// returns strOther with replacements from strOwn around nPosOwn

protected:
	CString m_strMark;

	static void GetUniqueIdx( const CString &strOwn, CDWordArray &adwIdx );
	static void FindMatchUnique ( const CString &strOwn, int &mn, CDWordArray &adwIdxOwn, int &ni,
						  const CString &strOther, int &mo, CDWordArray &adwIdxOther, int &oi );
};
