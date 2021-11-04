#pragma once
#include "DualTreeCtrl.h"
#include "DualTreeData.h"
#include "afxmt.h"
#include "afxtempl.h"

template<class ITYPE>		// ITYPE must be type of CDualTreeItem
class CDualTree :
	public CDualTreeCtrl
{
protected:
	typedef CDualTreeData<ITYPE> CItemDataList;
//	CDualTree(void);
	CDualTree(CItemDataList &data);
	~CDualTree(void);
	 HTREEITEM InsertItem( POSITION pos, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST );
	BOOL DeleteItem( HTREEITEM hItem );
	BOOL DeleteAllItems( );

public:
	ITYPE& GetItemData( HTREEITEM hItem );
	const ITYPE& GetItemData( HTREEITEM hItem ) const;
	BOOL IsSel( HTREEITEM hItem ) const { return GetItemData( hItem ).IsSel(); }
	void Sel( HTREEITEM hItem, BOOL b=TRUE ) { GetItemData( hItem ).Sel(b); SetParentSel(hItem,b); if (b) UnselChilds(hItem);}
	void UnselectAll();
	BOOL IsAnySel() { return m_bAnySel; }

private:
	CItemDataList &m_lItemData;

protected:
	BOOL m_bAnySel;

	void SetParentSel( HTREEITEM hItem, BOOL b=TRUE );	// 20100118
	BOOL IsChildSel( HTREEITEM hItem ) { return (hItem == NULL ? m_bAnySel : GetItemData( hItem ).IsChildSel() ); } // 20100118
	BOOL IsAnySel( HTREEITEM hItem ) { return (hItem == NULL ? m_bAnySel : GetItemData( hItem ).IsAnySel() ); } // 20100901
	BOOL IsChildSel2( HTREEITEM hItem ); // check all childs using IsAnySel
	void UnselChilds( HTREEITEM hItem ); // unselect all child items
	virtual void DrawTreeItem(CDC* pDC, LPNMTVCUSTOMDRAW lplvcd);
};

template<class ITYPE>
//CDualTree<ITYPE>::CDualTree(void)
CDualTree<ITYPE>::CDualTree(CItemDataList &data) : m_lItemData( data )
{
	m_bAnySel = FALSE;
}

template<class ITYPE>
CDualTree<ITYPE>::~CDualTree(void)
{
}

template<class ITYPE>
void CDualTree<ITYPE>::DrawTreeItem(CDC* pDC, LPNMTVCUSTOMDRAW lplvcd)
{
	NMTVCUSTOMDRAW nmtvcd = *lplvcd;
	HTREEITEM hItem = (HTREEITEM)lplvcd->nmcd.dwItemSpec;
	
	POSITION pItem = (POSITION)lplvcd->nmcd.lItemlParam;
	ASSERT( pItem != NULL );
	ITYPE &d = m_lItemData.GetItemData( pItem );

	nmtvcd.nmcd.rc.right = GetOffsLeft();
	nmtvcd.nmcd.rc.left = nmtvcd.iLevel * GetIndent();
	d.DrawTreeItemSide( pDC, *this, hItem, &nmtvcd, CDualTreeItem::common );
	lplvcd->clrText = nmtvcd.clrText;
	lplvcd->clrTextBk = nmtvcd.clrTextBk;

	nmtvcd.nmcd.rc.left = nmtvcd.nmcd.rc.right;
	nmtvcd.nmcd.rc.right = GetOffsRight();
	d.DrawTreeItemSide( pDC, *this, hItem, &nmtvcd, CDualTreeItem::left );

	nmtvcd.nmcd.rc.left = nmtvcd.nmcd.rc.right;
	nmtvcd.nmcd.rc.right = lplvcd->nmcd.rc.right;
	d.DrawTreeItemSide( pDC, *this, hItem, &nmtvcd, CDualTreeItem::right );
}

template<class ITYPE>
HTREEITEM CDualTree<ITYPE>::InsertItem( POSITION pos, HTREEITEM hParent /* = TVI_ROOT */, HTREEITEM hAfter /* = TVI_LAST */ )
{
//	ITYPE &d = m_lItemData.GetAt( pos );
	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = hAfter;
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText = LPSTR_TEXTCALLBACK;
	tvis.item.iImage = 0;
	tvis.item.iSelectedImage = 0;
	tvis.item.lParam = (LPARAM)pos;
	return CDualTreeCtrl::InsertItem( &tvis );
}

template<class ITYPE>
BOOL CDualTree<ITYPE>::DeleteItem( HTREEITEM hItem )
{
	if ( hItem == NULL )
		return FALSE;
	Sel( hItem, FALSE );
	POSITION pos = GetItemPos( hItem );
	if ( CDualTreeCtrl::DeleteItem( hItem ) )
	{
//		m_lItemData.DeleteItem( pos );
		return TRUE;
	}
	return FALSE;
}

template<class ITYPE>
BOOL CDualTree<ITYPE>::DeleteAllItems( )
{
	if ( CDualTreeCtrl::DeleteAllItems() )
	{
//		m_lItemData.DeleteAllItems();
		return TRUE;
	}
	return FALSE;
}

template<class ITYPE>
ITYPE& CDualTree<ITYPE>::GetItemData( HTREEITEM hItem )
{
	return m_lItemData.GetItemData( GetItemPos( hItem ) );
}

template<class ITYPE>
const ITYPE& CDualTree<ITYPE>::GetItemData( HTREEITEM hItem ) const
{
	return m_lItemData.GetItemData( GetItemPos( hItem ) );
}


template<class ITYPE>
void CDualTree<ITYPE>::UnselectAll()
{
	UnselChilds( NULL );
}

template<class ITYPE>
BOOL CDualTree<ITYPE>::IsChildSel2( HTREEITEM hItem )
{
	HTREEITEM hChild;
	if ( hItem == NULL )
		hChild = GetRootItem();
	else
		hChild = GetChildItem( hItem );
	while ( hChild != NULL )
	{
		if ( IsAnySel( hChild ) ) {
//            TRACE1( "  IsChildSel2 %s TRUE\n", GetItemData( hItem ).GetNameDebug() );
			return TRUE;
		}
		hChild = GetNextSiblingItem( hChild );
	}
//    TRACE1( "  IsChildSel2 %s FALSE\n", hItem == NULL ? _T("<root>") : GetItemData( hItem ).GetNameDebug() );
	return FALSE;
}

template<class ITYPE>
void CDualTree<ITYPE>::SetParentSel( HTREEITEM hItem, BOOL b )
{
//    TRACE2( "  SetParentSel %s %d\n", GetItemData( hItem ).GetNameDebug(), b );
	HTREEITEM hParent = GetParentItem( hItem );
	if ( hParent == NULL )
	{
		BOOL bNew = TRUE;
		if ( !b ) {
			bNew = IsChildSel2( hParent );
		}
		m_bAnySel = bNew;
		return;
	}

    BOOL bOld = IsChildSel( hParent );
	BOOL bNew = TRUE;
	if ( !b ) {
		bNew = IsChildSel2( hParent );
	}
	if ( bOld != bNew ) {
//        TRACE2( "  SetParentSel DIFF %d %d FALSE\n", bOld, bNew );
		GetItemData( hParent ).SetChildSel( bNew );
		if ( bNew )
			GetItemData( hParent ).Sel( FALSE );
		SetParentSel( hParent, bNew );
	}
}

template<class ITYPE>
void CDualTree<ITYPE>::UnselChilds( HTREEITEM hItem )
{
	if ( IsAnySel( hItem ) ) {	// 10200901
		HTREEITEM hChild;
		if ( hItem == NULL ) {
			hChild = GetRootItem();
			m_bAnySel = FALSE;
		} else {
			hChild = GetChildItem( hItem );
			GetItemData( hItem ).SetChildSel( FALSE );
		}
		while ( hChild != NULL )
		{
			GetItemData( hChild ).Sel( FALSE );
			UnselChilds( hChild );
			hChild = GetNextSiblingItem( hChild );
		}
	}
}


