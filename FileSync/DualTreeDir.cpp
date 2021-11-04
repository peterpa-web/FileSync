#include "StdAfx.h"
#include <math.h>
#include "DocFileSync.h"
#include "DragDropImpl.h"
#include "ViewDirItem.h"
#include "DualTreeDirData.h"

#include "DualTreeDir.h"

BEGIN_MESSAGE_MAP(CDualTreeDir, CDualTree<CViewDirItem>)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

//CDualTreeDir::CDualTreeDir(void)
CDualTreeDir::CDualTreeDir(CDualTreeDirData &data) : CDualTreeDirBase(data), m_data(data)
{
	m_bDrag = FALSE;
	m_bExpanding = FALSE;
	m_bLocked = FALSE;
	m_bBusyCursor = FALSE;
	m_dwTicksOptimize = 0;
}

CDualTreeDir::~CDualTreeDir(void)
{
}

BOOL CDualTreeDir::DeleteAllItems( )
{
	if ( !CDualTreeDirBase::DeleteAllItems() )
		return FALSE;

	if ( !m_data.DeleteAllItems() )
		return FALSE;
	return TRUE;
}


HTREEITEM CDualTreeDir::InsertItem( POSITION pos )
{
	CViewDirItem &d = m_data.GetItemData( pos );
	ASSERT( d.GetItemHandle() == NULL );
	POSITION posParent = d.GetItemParentPos();
	HTREEITEM hParent = TVI_ROOT;
	if ( posParent != NULL ) {
		CViewDirItem &dp = m_data.GetItemData( posParent );
//		TRACE2(" InsertItem %s/%s\n", dp.GetName(), d.GetName() );
		hParent = dp.GetItemHandle();
	}
	ASSERT( hParent != NULL );
	if ( hParent == NULL )
		return NULL; // can't insert
	HTREEITEM hAfter = m_data.GetPrevRealItem( pos );
	if ( hAfter == NULL )
		hAfter = TVI_FIRST;
	HTREEITEM hItem = CDualTreeDirBase::InsertItem( pos, hParent, hAfter );
	ASSERT( hItem != NULL );
	d.SetItemHandle( hItem );
	if ( d.GetIcon() != CDocFileSync::IconUnknown )
		SetItemImage( hItem, d.GetIcon(), d.GetIcon() ); 
	if ( d.HasDirIcon() )
	{
		if ( d.GetName() != _T("..") )
		{
//			TRACE3(" Insert %d %s exp=%d\n", hItem, d.GetName(), m_bExpandPart );
			TVITEM tvi;
			tvi.hItem = hItem;
			tvi.mask = TVIF_CHILDREN;
			tvi.cChildren = 1;
			SetItem( &tvi );
		}
	}
	else if ( d.HasArcIcon() )
	{
		TVITEM tvi;
		tvi.hItem = hItem;
		tvi.mask = TVIF_CHILDREN;
		tvi.cChildren = 1;
		SetItem( &tvi );
	}
	return hItem;
}

BOOL CDualTreeDir::DeleteItem( TREEPOS pos )
{
	// delete childs first
	TREEPOS posCh = GetFirstChildPos(pos);
	while ( posCh != NULL ) {
		TREEPOS posCh2 = GetNextSiblingPos(posCh);
		if ( !DeleteItem(posCh) )
			return FALSE;
		posCh = posCh2;
	}
	CViewDirItem &d = m_data.GetItemData(pos);
	TRACE2( "CDualTreeDir::DeleteItem %s %x\n", d.GetNameDebug(), pos );
	HTREEITEM hItem = d.GetItemHandle();
	if ( hItem != NULL ) {
//		ASSERT( d.IsDeleted() );
		if ( !DeleteItem( hItem ) )
			return FALSE;
		d.SetItemHandle( NULL );
	}
	return m_data.DeleteItem( pos );
}

void CDualTreeDir::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	if ((hItem == NULL))
		return;
	if ( m_bEnableClick &&
		(uFlags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) != 0 )
	{
		HTREEITEM hItemCaret = GetSelectedItem();
		if ( hItemCaret != NULL )
		{
			if ( (nFlags & MK_CONTROL) == MK_CONTROL )
			{
				SelectToggle(hItem);
			}
			else if ( (nFlags & MK_SHIFT) == MK_SHIFT && hItem != hItemCaret )
			{
				SelectRange(hItem);
				return;		// skipping update of caret
			}
			else
				SelectSingle(hItem);
		}
		else
			SelectSingle(hItem);
	}
	CDualTreeDirBase::OnLButtonDown(nFlags, point);
}

void CDualTreeDir::SelectSingle(HTREEITEM hItem)
{
	UnselectAll();
	Sel( hItem );
	Invalidate();
}

void CDualTreeDir::SelectToggle(HTREEITEM hItem)
{
	if ( IsSel( hItem ) )
		Sel( hItem, FALSE );
	else
		Sel( hItem );
	Invalidate();
}

void CDualTreeDir::SelectRange(HTREEITEM hItem)
{
	HTREEITEM hParent = GetParentItem(hItem);
	HTREEITEM hCaret = GetSelectedItem();
	HTREEITEM hParentCaret = GetParentItem(hCaret);
	if ( hParent != hParentCaret )
		return;
	HTREEITEM hChild = GetChildItem( hParentCaret );
	BOOL bCaret = FALSE;
	BOOL bCurr = FALSE;
	BOOL bSel = FALSE;
	while ( hChild != NULL )
	{
		if ( hChild == hItem )
			bCurr = TRUE;
		if ( hChild == hCaret )
			bCaret = TRUE;
		if ( !bSel && bCurr != bCaret )
		{
			bSel = TRUE;
		}
		GetItemData( hChild ).Sel( bSel );
		if ( bSel && bCurr == bCaret )
			bSel = FALSE;
		hChild = GetNextSiblingItem( hChild );
	}
	Invalidate();
}

//BOOL CDualTreeDir::ExpandAll( HTREEITEM hItem )
//{
//	m_bExpandAll = TRUE;
//	return ExpandAllPart( hItem );
//	return FALSE;
//}

void CDualTreeDir::ShowPopup( LPPOINT ppt )
{
	Select( m_hItemCurr, TVGN_CARET );
//	BOOL bDir = GetItemData(m_hItemCurr).IsDir();
	const CViewDirItem &d = GetItemData( m_hItemCurr );
	BOOL bDir = d.HasDirOrArcIcon(); 
	CMenu* pMenuPopup = ( bDir ? m_pMenuContextDir->GetSubMenu( 0 ) : m_pMenuContext->GetSubMenu( 0 ) );
	pMenuPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON,
		ppt->x, ppt->y, GetParentFrame() );
	TRACE0( "CDualTreeDir::ShowPopup ret\n" );
}

void CDualTreeDir::OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo)
{
	if ( (pTVDispInfo->item.mask & TVIF_TEXT) == 0 )
		return;

	// compute available text size
	HTREEITEM hItem = pTVDispInfo->item.hItem;

	int nLevel = 0;
	HTREEITEM hParentItem = GetParentItem( hItem );
	while ( hParentItem != NULL )
	{
		++nLevel;
		hParentItem = GetParentItem( hParentItem );
	}
	int nPix = ( m_nOffsLeft - 44 - nLevel * GetIndent() );
	if ( nPix < 0 )
		nPix = 0;
	int nChars = nPix / m_nCharWidth;
	if ( nChars >= pTVDispInfo->item.cchTextMax )
		nChars = pTVDispInfo->item.cchTextMax - 1;
	GetItemData(hItem).GetDispName(pTVDispInfo->item.pszText, nChars);
}


void CDualTreeDir::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	TRACE0( "CDualTreeDir::OnTvnBegindrag\n" );
	if ( m_bDrag )
	{
		*pResult = 0;
		return;
	}

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	UINT uFlags;
	HTREEITEM hItem = HitTest(pNMTreeView->ptDrag, &uFlags);

	if ((hItem == NULL))
	{
		*pResult = 0;
		return;
	}
	m_bDrag = TRUE;
	CViewDirItem &d = GetItemData( hItem );
	int nSide = m_nSide;
	if ( !d.IsPresent(nSide) )
		nSide = 1 - m_nSide;

	CString strName = d.GetDirEntry(nSide).GetName();
	CString strPath = d.GetParentDoc(nSide)->GetFullPathLnk( strName );

	CIDropSource* pdsrc = new CIDropSource;
	pdsrc->AddRef();
	CIDataObject* pdobj = new CIDataObject(pdsrc);
	pdobj->AddRef();
	// Init the supported format
	FORMATETC fmtetc = {0};
	fmtetc.cfFormat  = CF_HDROP;
	fmtetc.dwAspect  = DVASPECT_CONTENT;
	fmtetc.lindex    = -1;
	fmtetc.tymed     = TYMED_HGLOBAL;
	// Init the medium used
	STGMEDIUM medium = {0};
	medium.tymed     = TYMED_HGLOBAL;

	// medium.hGlobal = init to path
	SIZE_T dwBytes = sizeof(DROPFILES) + 2*strPath.GetLength()+4; //for 2*NULL
	medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE,dwBytes );
	BYTE* pMem = (BYTE*)GlobalLock(medium.hGlobal);
	memset(pMem, 0, dwBytes);
	LPDROPFILES pDF = (LPDROPFILES)pMem;
	TCHAR* pFiles = (TCHAR*)(pMem+sizeof(DROPFILES));
	pDF->pFiles = sizeof(DROPFILES);
#ifdef _UNICODE
	pDF->fWide = TRUE;
#else
	pDF->fWide = FALSE;
#endif
	_tcscpy_s(pFiles,strPath.GetLength()+1, strPath);
	GlobalUnlock(medium.hGlobal);

	// Add it to DataObject
	pdobj->SetData(&fmtetc,&medium,TRUE);   

	// Initiate the Drag & Drop
	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);
	if ( hr == DRAGDROP_S_DROP ) {
		TRACE0("DRAGDROP_S_DROP\n");
	} else if ( hr == DRAGDROP_S_CANCEL ) {
		TRACE0("DRAGDROP_S_CANCEL\n");
	} else {
		TRACE0("E_UNSPEC\n");
	}
	pdsrc->Release();
	pdobj->Release();
	*pResult = 0;
	m_bDrag = FALSE;
}

void CDualTreeDir::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( !m_bDrag && nFlags == MK_LBUTTON )
	{
		LRESULT res;
		NMTREEVIEW notify;
		notify.ptDrag = point;
		OnTvnBegindrag( (NMHDR*)(&notify), &res);
	}

	CDualTree<CViewDirItem>::OnMouseMove(nFlags, point);
}

HTREEITEM CDualTreeDir::GetFirstSel()
{
	if ( !m_bAnySel )
		return NULL;
	return GetFirstSelInt( GetRootItem() );
}

HTREEITEM CDualTreeDir::GetNextSel(HTREEITEM hItem)
{
	if ( hItem == NULL )
		return NULL;
	HTREEITEM hNext = GetNextSiblingItem(hItem);
	if ( hNext == NULL )
		return GetNextSel( GetParentItem(hItem) );
	return GetFirstSelInt( hNext );
}

//BOOL CDualTreeDir::IsAnySel()	// 10200910 del
//{
//	HTREEITEM hItem = GetFirstSel();
//	while ( hItem != NULL )
//	{
//		CViewDirItem &d = GetItemData( hItem );
//		if ( d.IsPresent(m_nSide) && d.IsAnySel() )
//			return TRUE;
//		hItem = GetNextSel( hItem );
//	}
//	return FALSE;
//}

BOOL CDualTreeDir::AllSelAreReady()
{
	HTREEITEM hItem = GetFirstSel();
	while ( hItem != NULL )
	{
		CViewDirItem &d = GetItemData( hItem );
		if ( (m_nSide == 2 || d.IsPresent( m_nSide )) &&
			(d.IsMarkPending() || d.IsDeleted() || d.IsMarkDirty()) )
			return FALSE;
		hItem = GetNextSel( hItem );
	}
	return TRUE;
}

HTREEITEM CDualTreeDir::GetFirstSelInt(HTREEITEM hItem)
{
	HTREEITEM hNext = hItem;
	while ( hNext != NULL )
	{
		CViewDirItem &d = GetItemData( hNext );
#ifdef _DEBUG
//			CString str = d.GetName();	// debug only
//			TRACE1("CDualTreeDir::GetFirstSelInt %s\n", str);
#endif
		if ( d.IsSel() )
			return hNext;

		if ( d.IsChildSel() ) {
			HTREEITEM hChild = GetChildItem( hNext );
			if ( hChild != NULL )
				return GetFirstSelInt( hChild );
		}
		hNext = GetNextSiblingItem(hNext);
	}
//	if ( hNext == NULL )
		return GetNextSel( GetParentItem(hItem) );

}


void CDualTreeDir::SelDiffChilds( int nSide, HTREEITEM hItem )
{
	HTREEITEM hChild = GetChildItem( hItem );
	HTREEITEM hVisible = GetNextVisibleItem( hItem );
	if ( hChild != hVisible ) { // not expanded
		CViewDirItem &d = GetItemData( hItem );
		if ( d.IsDiff(nSide) ) {
			Sel(hItem);
		}
		return;
	}

	while ( hChild != NULL )
	{
		CViewDirItem &d = GetItemData( hChild );
		if ( d.IsDiff(nSide) ) {
			HTREEITEM hChild2 = GetChildItem( hChild );
			if ( hChild2 != NULL )
				SelDiffChilds( nSide, hChild );
			else
				Sel(hChild);
		}
		hChild = GetNextSiblingItem( hChild );
	}
}

void CDualTreeDir::UpdRealItem(TREEPOS &pos)
{
	CViewDirItem &d = m_data.GetItemData(pos);
//	TRACE2( "CDualTreeDir::UpdRealItem %s %d\n", d.GetNameDebug(), d.IsDeleted() );
	if ( !d.IsDeleted() ) {
		HTREEITEM hItem = d.GetItemHandle();
		if ( hItem == NULL ) {
			ASSERT( !d.HasDirOrArcIcon() || (d.GetParentDoc(0) != NULL && d.GetParentDoc(1) != NULL) );
	//		TRACE1( " InsItem %s\n", d.GetNameDebug() );
			HTREEITEM hItem = InsertItem( pos );
			ASSERT( hItem != NULL );
		}
	}
	pos = d.GetNextSiblingPos();
}

void CDualTreeDir::UpdColumnSize()
{
	UpdCharsLeft(  m_data.GetFSCharCount(0) + 18 );
	UpdCharsRight( m_data.GetFSCharCount(1) + 18 );
}

TREEPOS CDualTreeDir::GetFirstChildPos( TREEPOS posParent ) const 
{ 
	return m_data.GetFirstChildPos( posParent ); 
}

TREEPOS CDualTreeDir::GetNextSiblingPos( TREEPOS pos ) const 
{ 
	return m_data.GetNextSiblingPos( pos ); 
}

#ifdef _DEBUG
const CString CDualTreeDir::GetItemNameDebug( TREEPOS pos )
{
	CString str = m_data.GetItemNameDebug(pos);
	if ( str[0] != '!' ) {
		CViewDirItem &d = m_data.GetItemData(pos);
		str = m_data.GetItemNameDebug(d.GetItemParentPos()) + _T("\\") + str;
	}
	return str;
}
#endif

void CDualTreeDir::InvalidateItem( TREEPOS pos )
{
	CViewDirItem &d = m_data.GetItemData(pos);
	if ( d.IsMarkDirty() )						// 2012-02-01
		return;
	HTREEITEM hItem = d.GetItemHandle();
	if ( hItem != NULL ) {
		if ( d.HasDirIcon() ) {
			TVITEM tvi;
			tvi.mask = TVIF_CHILDREN;
			tvi.hItem = hItem;
			if ( GetItem( &tvi ) ) {
				if ( d.GetLastChildPos() == NULL && tvi.cChildren == 1 ) {	// check for removing dummy "+"
					TRACE1( "CDualTreeDir::UpdRealItem EmptyDir %s\n", d.GetNameDebug() );
					d.ResetMarkPending();
					tvi.cChildren = 0;
					SetItem( &tvi );
				}
				else if ( d.GetLastChildPos() != NULL && tvi.cChildren == 0 ) {	// check for adding "+"
					TRACE1( "CDualTreeDir::UpdRealItem NonEmptyDir %s\n", d.GetNameDebug() );
					tvi.cChildren = 1;
					SetItem( &tvi );
				}
			}
		}
		CDualTreeDirBase::InvalidateItem( hItem );
	}
}

BOOL CDualTreeDir::HasDirIcon( TREEPOS pos ) const
{
	return m_data.GetItemData( pos ).HasDirIcon();
}

BOOL CDualTreeDir::IsArcExpand( TREEPOS pos ) const
{
	if ( pos == NULL ) 
		return FALSE;
	const CViewDirItem &d = m_data.GetItemData( pos );
	HTREEITEM hItem = d.GetItemHandle();
	return ( d.HasArcIcon() && hItem != NULL && GetChildItem( hItem ) == NULL );
}

BOOL CDualTreeDir::Expand( TREEPOS pos )
{
	TRACE1("CDualTreeDir::Expand %s\n", m_data.GetItemNameDebug( pos ));
	m_bExpanding = TRUE;
//	HTREEITEM hFirst = GetFirstVisibleItem();
//	ShowWindow( SW_HIDE );
	BOOL bRet = CDualTreeDirBase::Expand( m_data.GetItemData( pos ).GetItemHandle(), TVE_EXPAND );
//	EnsureVisible( hFirst );
//	ShowWindow( SW_NORMAL );
	m_bExpanding = FALSE;
	return bRet;
}

//BOOL CDualTreeDir::Collapse( TREEPOS pos )
//{
//	return Collapse( m_data.GetItemData( pos ).GetItemHandle() );
//}

//BOOL CDualTreeDir::Free( TREEPOS pos )
//{
//	HTREEITEM hItem = m_data.GetItemData( pos ).GetItemHandle();
//	if ( CDualTreeDirBase::Expand( hItem, TVE_COLLAPSE ) )
//	{
//		FreeItemRecursive( pos );
//		return TRUE;
//	}
//	return FALSE;
//}

void CDualTreeDir::FreeItemRecursive( TREEPOS pos, BOOL bDel )
{
	CViewDirItem &d = m_data.GetItemData( pos );
//	TRACE1( "CDualTreeDir::FreeItemRecursive %s\n", d.GetNameDebug() );
	// check dependent items first
	TREEPOS posChild = m_data.GetFirstChildPos( pos );
	while ( posChild != NULL )
	{
		TREEPOS posItem = posChild;
		posChild = m_data.GetNextSiblingPos( posChild );
		FreeItemRecursive( posItem, TRUE );
	}

	if ( bDel ) {
		HTREEITEM hItem = d.GetItemHandle();
		TRACE2( "CDualTreeDir::FreeItemRecursive %x %s\n", hItem, d.GetNameDebug() );
		d.FreeItem();
		VERIFY( CDualTreeDirBase::DeleteItem( hItem ) );
		d.SetItemHandle( NULL );
		m_data.DeleteItem( pos );
	}
	else {
		d.Invalidate();
	}
}

BOOL CDualTreeDir::EnsureVisible( TREEPOS pos )
{
	return EnsureVisible( m_data.GetItemData( pos ).GetItemHandle() );
}

TREEPOS CDualTreeDir::GetItemParentPos( HTREEITEM hItem ) const
{
	return GetItemData( hItem ).GetItemParentPos();
}

BOOL CDualTreeDir::IsItemVisible( TREEPOS pos )
{
	return CDualTreeDirBase::IsItemVisible( m_data.GetItemData( pos ).GetItemHandle() );
}

BOOL CDualTreeDir::LockWindowUpdate()
{
	m_bLocked = TRUE;
	TRACE0( "CDualTreeDir::LockWindowUpdate()\n" );
	return CDualTreeDirBase::LockWindowUpdate();
//	CDualTreeDirBase::SetRedraw( FALSE );
//	return TRUE;
}

void CDualTreeDir::UnlockWindowUpdate()
{
	m_bLocked = FALSE;
	CDualTreeDirBase::UnlockWindowUpdate();
//	CDualTreeDirBase::SetRedraw( TRUE );
	TRACE0( "CDualTreeDir::UnlockWindowUpdate()\n" );
}

void CDualTreeDir::SetBusyCursor( BOOL bBusy )
{
	if ( bBusy != m_bBusyCursor ) {
		m_bBusyCursor = bBusy;
		if ( !bBusy )
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
}

BOOL CDualTreeDir::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_bLocked ) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
		return TRUE;
	}
	if ( m_bBusyCursor ) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_APPSTARTING));
		return TRUE;
	}
	return CDualTreeDirBase::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CDualTreeDir::Optimize()
{
	DWORD dwTicksNow = GetTickCount();
	if ( dwTicksNow < m_dwTicksOptimize + 10000 )
		return FALSE;		// not now

	m_dwTicksOptimize = dwTicksNow;

	// update visible items
	HTREEITEM hItem = GetFirstVisibleItem();
	if ( hItem != NULL )	// touch parents
	{
		HTREEITEM hItemPar = GetParentItem( hItem );
		while ( hItemPar != NULL ) {
			GetItemData( hItemPar ).SetCurrentTicks( m_dwTicksOptimize );
			hItemPar = GetParentItem( hItemPar );
		}
	}
	int n = 0;
	HTREEITEM hItemBottom = GetBottomItem();
	while ( hItem != NULL )
	{
		GetItemData( hItem ).SetCurrentTicks( m_dwTicksOptimize );
		if ( hItem == hItemBottom )
			break;
		hItem = GetNextVisibleItem( hItem );
		if ( ++n > 60 ) {
			TRACE0( "CDualTreeDir::Optimize() > 60 visible items\n" );
			break;
		}
	}

	// check for outdated items
	TREEPOS pos = m_data.GetLastDirPos();
	while ( pos != NULL )
	{
		CViewDirItem &d = m_data.GetItemData( pos );
//		TRACE1( "CDualTreeDir::Optimize() check %s\n", d.GetName() );
		hItem = d.GetItemHandle();
		if ( hItem != NULL && d.HandleOutdated( m_dwTicksOptimize ) ) 
		{
			TRACE1( "CDualTreeDir::Optimize() outdated %s\n", d.GetName() );
			HTREEITEM hItemFirst = GetFirstVisibleItem();
			LockWindowUpdate();
			Collapse( hItem );
			d.SetMarkAll();
			d.SetMarkPending();
			InvalidateItem( pos );
			TREEPOS posChild = m_data.GetFirstChildPos( pos );
			while ( posChild != NULL )
			{
				TREEPOS posItem = posChild;
				posChild = m_data.GetNextSiblingPos( posChild );
				FreeItemRecursive( posItem, FALSE );
			}
			UnlockWindowUpdate();
			EnsureVisible( hItemFirst );
			m_dwTicksOptimize = 1;	// force continuing
			return TRUE;
		}
		pos = m_data.GetPrevDirPos( pos );
	}
	return FALSE;
}


void CDualTreeDir::Link()
{
	TRACE0( "CDualTreeDir::Link()\n" );
	HTREEITEM hItem1 = GetFirstSel();
	if ( hItem1 != NULL ) {
		TREEPOS pos1 = GetItemPos( hItem1 );
		Collapse( hItem1 );
		HTREEITEM hItem2 = GetNextSel( hItem1 );
		if ( hItem2 != NULL ) {
			Collapse( hItem2 );
			CViewDirItem &d1 = GetItemData( hItem1 );
			int nSide1 = d1.GetSide();
			CViewDirItem &d2 = GetItemData( hItem2 );
			if ( nSide1 == 1 ) {
				CDocDir *pDoc = d2.GetParentDoc( 1 );
				DOCPOS pos = d2.GetParentPos( 1 );
				d2.SetParentDoc( 1, d1.GetParentDoc( 1 ) );
				d2.SetParentPos( 1, d1.GetParentPos( 1 ) );
				d1.SetParentDoc( 1, pDoc );
				d1.SetParentPos( 1, pos );
				d1.SetMarkLink1();
				CDirEntry &de = d1.GetDirEntry( 1 );
				de.SetViewItemPos( pos1 ); 
			}
			else {
				CDocDir *pDoc = d2.GetParentDoc( 0 );
				DOCPOS pos = d2.GetParentPos( 0 );
				d2.SetParentDoc( 0, d1.GetParentDoc( 0 ) );
				d2.SetParentPos( 0, d1.GetParentPos( 0 ) );
				d1.SetParentDoc( 0, pDoc );
				d1.SetParentPos( 0, pos );
				d1.SetMarkLink2();
				CDirEntry &de = d1.GetDirEntry( 0 );
				de.SetViewItemPos( pos1 ); 
			}
			d1.SetMarkAll();
			if ( d1.IsDir() )
				d1.SetMarkPending();
			else
				d1.ResetMarkDirty();
			InvalidateItem( pos1 );
			TREEPOS pos2 = GetItemPos( hItem2 );
			FreeItemRecursive( pos2, TRUE );
			ASSERT(d1.IsPresent(0));
			ASSERT(d1.IsPresent(1));
			FreeItemRecursive( pos1, FALSE );
		}
	}
	TRACE0( "CDualTreeDir::Link() done\n" );
}

int CDualTreeDir::GetSelFiles( int nSide )
{
	int nCount = 0;
	HTREEITEM hItem = GetFirstSel();
	while ( hItem != NULL ) {
		nCount += GetFileCount( nSide, hItem );
		hItem = GetNextSel( hItem );
	}
	return nCount;
}

int CDualTreeDir::GetFileCount( int nSide, HTREEITEM hItem )
{
	int nCount = 0;
	HTREEITEM hChild = GetChildItem( hItem );
	if ( hChild == NULL ) {
		CViewDirItem &d = GetItemData( hItem );
		if ( d.IsPresent(nSide) && !d.IsDir() ) {
			return 1;
		}
	}
	while ( hChild != NULL )
	{
		CViewDirItem &d = GetItemData( hChild );
		if ( d.IsPresent(nSide) ) {
			nCount += GetFileCount( nSide, hChild );
		}
		hChild = GetNextSiblingItem( hChild );
	}
	return nCount;
}
