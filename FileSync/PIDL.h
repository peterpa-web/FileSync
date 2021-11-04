#pragma once

class CPIDL
{
public:
	CPIDL(void);
	~CPIDL(void);
	void Empty();
	static CString ToString(LPITEMIDLIST pidl);
	CString ToString() { return ToString(m_pidl); }
	BOOL FromString( LPTSTR path );
	BOOL FromCSIDL( int csidl );

	operator LPITEMIDLIST() const { return m_pidl; }
	void operator =( LPITEMIDLIST pidl ) { Empty(); m_pidl = pidl; }
	void operator =( const CPIDL& pidl );	// create copy of pidl

protected:
	static LPMALLOC s_lpMalloc;

	LPITEMIDLIST m_pidl;
};
