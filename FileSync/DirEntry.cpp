#include "StdAfx.h"
#include "DocDir.h"

#include "DirEntry.h"


CDirEntry::CDirEntry()
{
	m_fs = 0;
	m_crc = 0;
	m_flags = 0;
	m_pd = NULL;
	m_posViewItem = NULL;
}

CDirEntry::CDirEntry( const CDirEntry &src )
{
	ASSERT( src.m_pd == NULL ); // otherwise delete src would delete that obj, too !!!
	m_strName = src.m_strName;
	m_dt	  = src.m_dt;
	m_fs	  = src.m_fs;
	m_crc	  = src.m_crc;
	m_flags	  = src.m_flags;
	m_pd	  = NULL;
	m_posViewItem = NULL;
}

CDirEntry::CDirEntry( BOOL bDir, const CString &name, BOOL bZip /* = FALSE */ )
{
	m_strName = name;
	m_fs = 0;
	m_crc = 0;
	m_flags = (bDir ? Dir : 0) | (bZip ? Arch|NoExtr : 0);
	m_pd = NULL;
	m_posViewItem = NULL;
}

CDirEntry::CDirEntry( BOOL bDir, 
						const CString &name, 
						BOOL bRO,
						CTime dt, 
						__int64 fs, /* = 0 */
						ULONG crc, /* = 0 */
						BOOL bZip /* = FALSE */ )
{
	m_strName = name;
	m_dt = dt;
	m_fs = fs;
	m_crc = crc;
	m_flags = (bDir ? Dir : 0) | (bZip ? Arch|NoExtr : 0);
	if ( bRO )
		m_flags |= RO;
	m_pd = NULL;
	m_posViewItem = NULL;
}

CDirEntry::~CDirEntry()
{
	ASSERT( m_posViewItem == NULL );
//	delete m_pd;
	if ( m_pd != NULL )
		m_pd->Delete();
}

void CDirEntry::Copy( const CDirEntry &src, BOOL bPhysCopy )
{
	ASSERT( src.IsDir() == IsDir() );
	ASSERT( !src.IsDel() );
//	ASSERT( !src.IsModif() );
//	ASSERT( !bPhysCopy || !src.IsNoExtr() );
	if ( IsDir() )
	{
		ASSERT( !m_strName.IsEmpty() );
		if ( m_pd != NULL )
		{
			ASSERT( !m_pd->GetName().IsEmpty() );
			m_pd->UpdPathName();
		}
	}
	else
	{
//		ASSERT( src.pd == NULL );
//		ASSERT( pd == NULL );
		m_strName = src.m_strName;
		m_dt = src.m_dt;
		m_fs = src.m_fs;
//		pd = NULL;
	}
	m_crc = src.m_crc;
	m_flags &= ~Del;
	if ( IsArch() && bPhysCopy )
		SetNoExtr( FALSE );
}

BOOL CDirEntry::CopyAttr( const CDirEntry &src )
{
	ASSERT( m_strName == src.m_strName );
	ASSERT( IsDir() == src.IsDir() );
	if ( m_dt != src.m_dt || IsDel() || IsRO() != src.IsRO() || 
		(!IsDir() && (m_fs != src.m_fs)) )
	{
		TRACE3( "  CopyAttr %4.4x %4.4x %s\n", m_flags, src.GetFlags(), m_strName );
		m_dt = src.m_dt;
		m_fs = src.m_fs;
		m_crc = 0;
		SetRO( src.IsRO() );
		SetMark();
		if ( IsDel() )
		{
			SetDel( FALSE );
			SetCopied();
		}
		return TRUE;
	}
	return FALSE;
}

int CDirEntry::Compare( const CDirEntry &other ) const
{
	if ( IsDir() == other.IsDir() )
	{
		int res = m_strName.CompareNoCase( other.m_strName );
		if ( res != 0 )
			return res;
		return m_strName.Compare( other.m_strName );
	}

	if ( IsDir() )
		return -1;

	return 1;
}
