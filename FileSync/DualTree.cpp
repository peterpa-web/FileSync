#include "StdAfx.h"
#include "DualTree.h"

//IMPLEMENT_DYNAMIC(CDualTree, CDualTreeCtrl)

template<class ITYPE>
CDualTree<ITYPE>::CDualTree(void)
{
}

template<class ITYPE>
CDualTree<ITYPE>::~CDualTree(void)
{
}

template<class ITYPE>
void CDualTree<ITYPE>::DrawTreeItem(CDC* pDC, LPNMTVCUSTOMDRAW lplvcd)
{
	NMTVCUSTOMDRAW nmtvcd = *lplvcd;
	nmtvcd.nmcd.rc.right = GetOffsLeft();
	nmtvcd.nmcd.rc.left = nmtvcd.iLevel * GetIndent();
//	m_pView->DrawTreeItemSide( pDC, *this, &nmtvcd, common );
	lplvcd->clrText = nmtvcd.clrText;
	lplvcd->clrTextBk = nmtvcd.clrTextBk;

	nmtvcd.nmcd.rc.left = nmtvcd.nmcd.rc.right;
	nmtvcd.nmcd.rc.right = GetOffsRight();
//	m_pView->DrawTreeItemSide( pDC, *this, &nmtvcd, left );

	nmtvcd.nmcd.rc.left = nmtvcd.nmcd.rc.right;
	nmtvcd.nmcd.rc.right = lplvcd->nmcd.rc.right;
//	m_pView->DrawTreeItemSide( pDC, *this, &nmtvcd, right );
}

template<class ITYPE>
HTREEITEM CDualTree<ITYPE>::AddItem( const ITYPE &d, HTREEITEM hParent /* = TVI_ROOT */ )
{
	POSITION pos = m_lItemData.AddTail( d );
//	HTREEITEM hItem = InsertItem( d.GetName(), 0, 0, hParent );	// LPSTR_TEXTCALLBACK
//	HTREEITEM hItem = InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, 
//		d.GetName(), 0, 0, 0, 0, (LPARAM)pos, hParent, TVI_LAST );
	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText = (LPTSTR)(LPCTSTR)d.GetName();
	tvis.item.iImage = 0;
	tvis.item.iSelectedImage = 0;
//	tvis.item.state = nState;
//	tvis.item.stateMask = nStateMask;
	tvis.item.lParam = (LPARAM)pos;
	HTREEITEM hItem = InsertItem( &tvis );
	if ( d.GetIcon() == CDocFileSync::IconDir )
	{
		SetItemImage( hItem, d.GetIcon(), d.GetIcon() ); 
		if ( d.GetName() != _T("..") )
		{
			TVITEM tvi;
			tvi.hItem = hItem;
			tvi.mask = TVIF_CHILDREN;
			tvi.cChildren = 1;
			SetItem( &tvi );
		}
	}
	return hItem;
}
