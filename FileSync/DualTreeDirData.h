#pragma once
#include "DualTreeData.h"
#include "ViewDirItem.h"

typedef CDualTreeData<CViewDirItem> CDualTreeDirDataBase;

class CDualTreeDirData :
	public CDualTreeDirDataBase
{
public:
	CDualTreeDirData(void);
	~CDualTreeDirData(void);
	TREEPOS CreateItem( const CViewDirItem &d, POSITION posAfter ); // create virt. tree entry
	HTREEITEM GetPrevRealItem( TREEPOS pos );
	BOOL DeleteItem( TREEPOS pos );
//	TREEPOS GetNextSiblingPos( TREEPOS pos ) const { TREEPOS posNxt = CDualTreeDirDataBase::GetAt(pos).GetNextSiblingPos(); ASSERT(posNxt != pos); return posNxt; }
	TREEPOS GetNextSiblingPos( TREEPOS pos ) const;
//	TREEPOS GetPrevSiblingPos( TREEPOS pos ) const { TREEPOS posPre = CDualTreeDirDataBase::GetAt(pos).GetPrevSiblingPos(); ASSERT(posPre != pos); return posPre; }
	TREEPOS GetPrevSiblingPos( TREEPOS pos ) const;
	BOOL DeleteAllItems( );
	void ResetAllDocRefs( int nSide );
	void UpdRealItems(TREEPOS posStart);
	__int64 GetMaxFileSize(int nSide) { return m_nMaxFileSize[nSide]; }
	void SetMaxFileSize(int nSide, __int64 nMaxFileSize) { m_nMaxFileSize[nSide] = nMaxFileSize; }
	BOOL IncrMaxFileSize(int nSide, __int64 nMaxFileSize) { if (m_nMaxFileSize[nSide] < nMaxFileSize) { m_nMaxFileSize[nSide] = nMaxFileSize; return TRUE; } else return FALSE; }
	int GetFSCharCount( int nSide ) const;
//	TREEPOS GetFirstRootPos() const { return CDualTreeDirDataBase::GetHeadPosition(); } use GetFirstChildPos(NULL)
	TREEPOS GetFirstChildPos( TREEPOS posParent ) const;
	TREEPOS GetLastChildPos( TREEPOS posParent ) const;
	TREEPOS GetLastDirPos() const;
	TREEPOS GetPrevDirPos( TREEPOS pos ) const;

#ifdef _DEBUG
	const CString GetItemNameDebug( TREEPOS pos );
	void Dump() const;
#endif

protected:
	__int64 m_nMaxFileSize[2];
	POSITION m_posLastRoot;

	void SetLastChildPos( TREEPOS posParent, TREEPOS pos ) {
		if ( posParent != NULL ) 
			GetAt(posParent).SetLastChildPos( pos ); 
		else
			m_posLastRoot = pos;
	}
};
