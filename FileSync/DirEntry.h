#pragma once
#include "DualTreeData.h"

class CDocDir;

class CDirEntry
{
public:
	CDirEntry();
	CDirEntry( const CDirEntry &src );
	CDirEntry( BOOL bDir, const CString &name, BOOL bZip=FALSE );
	CDirEntry( BOOL bDir, const CString &name, BOOL bRO, CTime dt, __int64 fs=0, ULONG crc=0, BOOL bZip=FALSE );
	~CDirEntry();
	int Compare ( const CDirEntry &other ) const;
	void Copy( const CDirEntry &src, BOOL bPhysCopy=TRUE );
	BOOL CopyAttr( const CDirEntry &src );
	const CString & GetName() const { return m_strName; }
	void SetName( const CString &str ) { m_strName = str; }
	const CTime & GetDateTime() const { return m_dt; }
	void SetDateTime( const CTime &t ) { m_dt = t; }
	const __int64 & GetFileSize() const { return m_fs; }
	void SetFileSize( const __int64 &s ) { m_fs = s; }
	const ULONG & GetCRC() const { return m_crc; }
	void SetCRC( ULONG c ) { m_crc = c; }
	CDocDir * GetDoc() const { return m_pd; }
	void SetDoc( CDocDir *p ) { m_pd = p; }
	enum Flags{	// masks in flags
		Dir		= 0x1000,	// is directory
		RO		= 0x0100,	// is read only
		Del		= 0x0001,	// is deleted or dummy
		Mark	= 0x0002,	// mark for zip and merge processing
		Modif	= 0x0004,	// is modified
		Copied  = 0x0008,	// is copied
		Arch	= 0x0010,	// is in archive (zip)
		NoExtr	= 0x0020,	// not extracted yet
		All		= 0x1137
	};
#ifdef _DEBUG
	DWORD GetFlags() const { return m_flags; }
#endif
	BOOL IsAny( DWORD mask ) const { return (m_flags & mask) != 0; }
	BOOL AreAll( DWORD mask ) const { return (m_flags & mask) == mask; }
	BOOL IsNotAny( DWORD mask ) const { return (m_flags & mask) == 0; }
	BOOL IsDir() const { return (m_flags & Dir) != 0; }
	BOOL IsZip() const;		// see DocDirZipRoot.cpp
	BOOL IsDel() const { return (m_flags & Del) != 0; }
	BOOL IsPresent() const { return (m_flags & Del) == 0; } 
	BOOL IsPhysPresent() const { return (m_flags & (Del | NoExtr)) == 0; } 
	BOOL IsMarked() const { return (m_flags & Mark) != 0; }
	BOOL IsModif() const { return (m_flags & Modif) != 0; }
	BOOL IsCopied() const { return (m_flags & Copied) != 0; }
	BOOL IsArch() const { return (m_flags & Arch) != 0; }
	BOOL IsNoExtr() const { return (m_flags & NoExtr) != 0; }
	BOOL IsRO() const { return (m_flags & RO) != 0; }
	void SetFlags( DWORD f ) { ASSERT( m_flags < 0x2000 ); m_flags |= f; }
	void SetFlags( DWORD f, BOOL b ) { ASSERT( m_flags < 0x2000 ); m_flags = (b ? m_flags | f : m_flags & ~f); }
	void SetDir( BOOL b = TRUE ) { SetFlags( Dir, b ); }
	void SetDel( BOOL b = TRUE ) { SetFlags( Del, b ); }
	void SetMark( BOOL b = TRUE ) { SetFlags( Mark, b ); }
	void SetModif( BOOL b = TRUE ) { SetFlags( Modif, b ); }
	void SetCopied( BOOL b = TRUE ) { SetFlags( Copied, b ); }
	void SetArch( BOOL b = TRUE ) { SetFlags( Arch, b ); }
	void SetNoExtr( BOOL b = TRUE ) { SetFlags( NoExtr, b ); }
	void SetRO( BOOL b = TRUE ) { SetFlags( RO, b ); }
	void ResetFlags( DWORD mask ) { m_flags &= ~mask; }
	// 20100527
	void SetViewItemPos( TREEPOS pos ) { m_posViewItem = pos; }
	TREEPOS GetViewItemPos() { return m_posViewItem; }

protected:
	CString m_strName;
	__int64 m_fs;
	CTime m_dt;
	ULONG m_crc;
	DWORD m_flags;
	CDocDir *m_pd;
	// 20100527
	TREEPOS m_posViewItem;
};

