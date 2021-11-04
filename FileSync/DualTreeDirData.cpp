#include "StdAfx.h"
#include <math.h>
#include "DocFileSync.h"

#include "DualTreeDirData.h"


CDualTreeDirData::CDualTreeDirData(void)
{
	m_nMaxFileSize[0] = 0;
	m_nMaxFileSize[1] = 0;
	m_posLastRoot = NULL;
}

CDualTreeDirData::~CDualTreeDirData(void)
{
	TRACE1( "CDualTreeDirData::~CDualTreeDirData %x\n", this );
}

BOOL CDualTreeDirData::DeleteAllItems( )
{
	CSingleLock singleLock(&m_critSection, TRUE);
	TREEPOS pos = GetHeadPosition();
	while ( pos != NULL ) {
		CViewDirItem &d = GetNext(pos);
		if ( d.IsValid(0) )
			d.ResetDocRef(0);
		if ( d.IsValid(1) )
			d.ResetDocRef(1);
	}
	if ( !CDualTreeDirDataBase::DeleteAllItems() )
		return FALSE;

	m_nMaxFileSize[0] = 0;
	m_nMaxFileSize[1] = 0;
	m_posLastRoot = NULL;
	return TRUE;
}

void CDualTreeDirData::ResetAllDocRefs( int nSide )
{
	if ( nSide == 2 ) {
		ResetAllDocRefs( 0 );
		ResetAllDocRefs( 1 );
	}
	else {
		TRACE1( "CDualTreeDirData::ResetAllDocRefs %d\n", nSide );
		TREEPOS pos = GetHeadPosition();
		while ( pos != NULL ) {
			CViewDirItem &d = GetNext(pos);
			if ( d.IsValid(nSide) )
				d.ResetDocRef(nSide);
		}
		m_nMaxFileSize[nSide] = 0;
	}
}

TREEPOS CDualTreeDirData::CreateItem( const CViewDirItem &d, TREEPOS posAfter )
{
	CSingleLock singleLock(&m_critSection, TRUE);
	TRACE1( "CDualTreeDirData::CreateItem %s\n", d.GetName() );
	TREEPOS pos;
	TREEPOS posParent = d.GetItemParentPos();
	TREEPOS posNext = NULL; // locate next before insertion
	if ( posAfter == NULL )	{	// insert before 1st child / directly after parent
		posNext = GetFirstChildPos(posParent);
		if ( posParent != NULL ) {
			if ( posNext == NULL )
				pos = InsertAfter( posParent, d );
			else
				pos = InsertBefore( posNext, d );
		}
		else {
			pos = AddHead( d );
		}
	} else {
		CViewDirItem &da = GetAt(posAfter);
		posNext = da.GetNextSiblingPos();
		if ( posNext != NULL ) {
			pos = InsertBefore( posNext, d );
		} else {
			TREEPOS posNextParent = NULL;
			TREEPOS posUpper = posParent;
			while ( posNextParent == NULL && posUpper != NULL ) {	// try all upper parents
				posNextParent = GetAt(posUpper).GetNextSiblingPos();
				posUpper = GetAt(posUpper).GetItemParentPos();
			}
			if ( posNextParent != NULL )
				pos = InsertBefore( posNextParent, d );
			else
				pos = AddTail( d );
		}
	}
	// adjust chaining
	CViewDirItem &di = GetAt(pos);
	di.SetPrevSiblingPos( posAfter );
	di.SetNextSiblingPos( posNext );
	if ( di.IsPresent(0) )
		di.GetDirEntry(0).SetViewItemPos(pos);
	if ( di.IsPresent(1) )
		di.GetDirEntry(1).SetViewItemPos(pos);
	if ( posNext != NULL ) {
		GetAt(posNext).SetPrevSiblingPos( pos );
	} else {
		SetLastChildPos( posParent, pos );
	}
	if ( posAfter != NULL )
		GetAt(posAfter).SetNextSiblingPos( pos );
	return pos;
}

HTREEITEM CDualTreeDirData::GetPrevRealItem( TREEPOS pos )
{
	HTREEITEM hItem = NULL;
	CViewDirItem &dStart = GetAt( pos );
	TREEPOS posPrev = dStart.GetPrevSiblingPos();
	while ( hItem == NULL && posPrev != NULL )
		// skipping pure virtual items
	{
		CViewDirItem &d = GetAt( posPrev );
		hItem = d.GetItemHandle();
		posPrev = d.GetPrevSiblingPos();
	}
	return hItem;
}

BOOL CDualTreeDirData::DeleteItem( TREEPOS pos )
{
	CSingleLock singleLock(&m_critSection, TRUE);

	CViewDirItem &d = GetAt(pos);
	TRACE1( "CDualTreeDirData::DeleteItem %s\n", d.GetNameDebug() );
	ASSERT( d.GetLastChildPos() == NULL );	// no childs allowed
	ASSERT( d.GetItemHandle() == NULL );	// no real item allowed
//	ASSERT( d.IsDeleted() );	// no doc refs are allowed
	if ( d.IsValid(0) )
		d.ResetDocRef(0);
	if ( d.IsValid(1) )
		d.ResetDocRef(1);
	TREEPOS posPrev = d.GetPrevSiblingPos();
	TREEPOS posNext = d.GetNextSiblingPos();
	if ( posPrev != NULL ) {
		CViewDirItem &db = GetAt(posPrev);
		db.SetNextSiblingPos( posNext );
		d.SetPrevSiblingPos( NULL );
	}
	if ( posNext != NULL ) {
		CViewDirItem &dn = GetAt(posNext);
		dn.SetPrevSiblingPos( posPrev );
		d.SetNextSiblingPos( NULL );
	} else {
		TREEPOS posParent = d.GetItemParentPos();
		SetLastChildPos( posParent, posPrev );
	}

	return CDualTreeDirDataBase::DeleteItem( pos );
}

TREEPOS CDualTreeDirData::GetNextSiblingPos( TREEPOS pos ) const
{
	const CViewDirItem &d = GetAt(pos);
	TREEPOS posNxt = d.GetNextSiblingPos();
#ifdef _DEBUG
	if ( posNxt != NULL ) {
		ASSERT( d.GetItemParentPos() == GetAt(posNxt).GetItemParentPos() );
	}
	ASSERT(posNxt != pos);
#endif
	return posNxt;
}

TREEPOS CDualTreeDirData::GetPrevSiblingPos( TREEPOS pos ) const
{ 
	const CViewDirItem &d = GetAt(pos);
	TREEPOS posPre = d.GetPrevSiblingPos();
#ifdef _DEBUG
	if ( posPre != NULL ) {
		ASSERT( d.GetItemParentPos() == GetAt(posPre).GetItemParentPos() );
	}
	ASSERT(posPre != pos);
#endif
	return posPre;
}

TREEPOS CDualTreeDirData::GetFirstChildPos( TREEPOS posParent ) const
{
	if ( posParent == NULL )
		return GetHeadPosition();
	// if present the next item should be the 1st child
	// checking by parent pos comparison
	TREEPOS posNext = posParent;
	GetNext( posNext );
	if ( posNext != NULL ) {
		if ( posParent != GetAt( posNext ).GetItemParentPos() ) {
#ifdef _DEBUG
//			TRACE1( "!  Par=%s\n", GetAt( posParent ).GetName() );
//			TRACE1( "!  Nxt=%s\n", GetAt( posNext ).GetName() );
//			TREEPOS posNP = GetAt( posNext ).GetItemParentPos();
//			TRACE1( "!  NP =%s\n", posNP == NULL ? _T("NULL") : GetAt( posNP ).GetName() );
//			Dump();
#endif
			return NULL;
		}
	}
	return posNext;
}

TREEPOS CDualTreeDirData::GetLastChildPos( TREEPOS posParent ) const
{
	if ( posParent == NULL )
		return m_posLastRoot;
		
	return GetAt( posParent ).GetLastChildPos();
}

int CDualTreeDirData::GetFSCharCount( int nSide ) const 
{ 
	int n = 1 + (int)( 1.334 * log10( (double)(m_nMaxFileSize[nSide]+1) ) );
	if ( n < 3 )
		return 3;
	return n;
}

TREEPOS CDualTreeDirData::GetLastDirPos() const
{
	TREEPOS pos = GetTailPosition();
	while ( pos != NULL )
	{
		if ( GetAt( pos ).IsDir() )
			break;
		GetPrev( pos );
	}
	return pos;
}

TREEPOS CDualTreeDirData::GetPrevDirPos( TREEPOS pos ) const
{
	GetPrev( pos );
	while ( pos != NULL )
	{
		if ( GetAt( pos ).IsDir() )
			break;
		GetPrev( pos );
	}
	return pos;
}


#ifdef _DEBUG
const CString CDualTreeDirData::GetItemNameDebug( TREEPOS pos )
{
	if ( pos == NULL )
		return _T("!NULL");

	TREEPOS posT = GetHeadPosition();
	while ( posT != NULL ) {
		if ( posT == pos )
			return GetAt(pos).GetNameDebug();
		GetNext(posT);
	}
	return _T("!BAD POS");
}

void CDualTreeDirData::Dump( ) const
{
	TREEPOS pos = GetHeadPosition();
	while ( pos != NULL ) {
		TREEPOS posX = pos;
		const CViewDirItem &d = GetNext(pos);
		TRACE( _T("> %x %x %x %x %x %s\n"), posX, d.GetItemParentPos(), d.GetPrevSiblingPos(), d.GetNextSiblingPos(), d.GetLastChildPos(), d.GetName() );
	}
}

#endif
