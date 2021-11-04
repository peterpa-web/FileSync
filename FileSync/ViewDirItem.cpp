#include "StdAfx.h"
#include "DocDir.h"
#include "DualTreeCtrl.h"
#include "resource.h"

#include "ViewDirItem.h"

CString CViewDirItem::s_strDel( _T("\\del/") );

CViewDirItem::CViewDirItem()
{
}

CViewDirItem::CViewDirItem( TREEPOS posParent, CDocDir* ppdLeft, CDocDir* ppdRight )
{
	m_nIcon = CDocFileSync::IconUnknown;
	m_nMark = markDirty;
	s[0].ppd = ppdLeft;
	s[1].ppd = ppdRight;
	s[0].pos = NULL;
	s[1].pos = NULL;

	m_hItem = NULL;
	m_posParent = posParent;
	m_posPrevSibling = NULL;
	m_posNextSibling = NULL;
	m_posLastChild = NULL;
	m_dwTicksShow = 0;
}

CViewDirItem::~CViewDirItem()
{
//	ASSERT( m_posNextSibling == NULL );
#ifdef _DEBUG
	m_strNameDebug.Empty();
#endif
}

const CString & CViewDirItem::GetName() const
{
//	if ( IsMarkLink() ) {
//		if ( IsMarkLink1() ) {
//			return GetDirEntry( 0 ).GetName() + _T("/") + GetDirEntry( 1 ).GetName();
//		}
//		else {
//			return GetDirEntry( 1 ).GetName() + _T("/") + GetDirEntry( 0 ).GetName();
//		}
//	}
	if ( s[0].pos != NULL )
		return GetDirEntry( 0 ).GetName();
	if ( s[1].pos != NULL )
		return GetDirEntry( 1 ).GetName();
	return s_strDel;
}

#ifdef _DEBUG
const CString & CViewDirItem::GetNameDebug()
{
	if ( !IsDeleted() || m_strNameDebug.IsEmpty() )
		m_strNameDebug.SetString( GetName() );
	return m_strNameDebug;
}
#endif

CString CViewDirItem::GetFullPath() const
{
	if ( s[0].pos != NULL )
		return GetParentDoc( 0 )->GetFullPath(GetDirEntry( 0 ).GetName());
	if ( s[1].pos != NULL )
		return GetParentDoc( 1 )->GetFullPath(GetDirEntry( 1 ).GetName());;
	return s_strDel;
}

void CViewDirItem::GetDispName( LPWSTR pDest, int nSize ) const
{
	if ( IsDeleted() ) {
		_tcscpy_s( pDest, nSize, s_strDel );
		return;
	}

	CString strDispName;
	if ( IsMarkLink() ) {
		if ( IsMarkLink1() ) {
			strDispName = GetDirEntry( 0 ).GetName() + _T("/") + GetDirEntry( 1 ).GetName();
		}
		else {
			strDispName = GetDirEntry( 1 ).GetName() + _T("/") + GetDirEntry( 0 ).GetName();
		}
	}
	else {
		const CDirEntry *pde;
		if ( s[0].pos != NULL )
			pde = &GetDirEntry( 0 );
		else
			pde = &GetDirEntry( 1 );

		strDispName = pde->GetName();
	}
	if ( strDispName.GetLength() > nSize )
	{
		if ( nSize < 1 ) {
			strDispName = _T("*");
			nSize = 1;
		} else
			strDispName = strDispName.Left( nSize-1 ) + _T("*");
	}
	_tcscpy_s( pDest, nSize+1, strDispName );
}

const BOOL CViewDirItem::IsPresent( int nSide ) const
{
//	ASSERT( !IsMarkDel() );
	if ( s[nSide].pos == NULL )
		return FALSE;

	return GetDirEntry( nSide ).IsPresent();
}

const BOOL CViewDirItem::IsDir() const
{
	if ( s[0].pos != NULL )
		return GetDirEntry( 0 ).IsDir();
	if ( s[1].pos != NULL )
		return GetDirEntry( 1 ).IsDir();
	ASSERT( !HasUnknownIcon() );
	return HasDirOrArcIcon();
}

const BOOL CViewDirItem::HasDirIcon() const
{
	return ( GetIcon() == CDocFileSync::IconDir );
}

const BOOL CViewDirItem::HasTextIcon() const
{
	return ( GetIcon() == CDocFileSync::IconText );
}

const BOOL CViewDirItem::HasArcIcon() const
{
	return ( GetIcon() == CDocFileSync::IconZipRoot || 
		     GetIcon() == CDocFileSync::IconIsoRoot );
}

const BOOL CViewDirItem::HasDirOrArcIcon() const
{
	return ( GetIcon() == CDocFileSync::IconDir ||
			 GetIcon() == CDocFileSync::IconZipRoot ||
		     GetIcon() == CDocFileSync::IconIsoRoot );
}

const BOOL CViewDirItem::HasUnknownIcon() const
{
	return ( GetIcon() == CDocFileSync::IconUnknown );
}

const int CViewDirItem::GetSideMarked() const
{
	if ( IsSingle() ) {
		if ( IsMarkSingle(left) )
			return left;
		if ( IsMarkSingle(right) )
			return right;
		if ( IsPresent(left) )
			return left;
		return right;
	}
	return common;
}

const CTime & CViewDirItem::GetDateTime( int nSide ) const
{
	ASSERT( !IsDeleted() );
	ASSERT_VALID( GetParentDoc( nSide ) );
	return GetDirEntry( nSide ).GetDateTime();
}

CString CViewDirItem::GetDateTimeStr( int nSide ) const
{
	if ( IsDeleted() )
		return _T("(del)");
	CDocFileSync *pd = GetDirEntry( nSide ).GetDoc();
//	if ( pd != NULL && GetParentDoc( nSide )->IsModified() )		2011/03/18
	if ( pd != NULL && pd->IsModified() )
		return _T("(modif.)");
	if ( GetDateTime(nSide) == 0 )
		return _T("00.00.00 00:00:00");
	if ( GetDirEntry( nSide ).IsRO() )
		return GetDateTime(nSide).Format( _T("%d.%m.%yR%H:%M:%S") );
	return GetDateTime(nSide).Format( _T("%d.%m.%y %H:%M:%S") );
}

//const __int64 & CViewDirItem::GetFileSize( int nSide ) const
__int64 CViewDirItem::GetFileSize( int nSide ) const
{
#pragma warning( push )
#pragma warning( disable : 4172 )
	if ( IsPresent( nSide ) )
		return GetDirEntry( nSide ).GetFileSize();
	return 0;
#pragma warning( pop ) 
}

void CViewDirItem::SetFileSize( int nSide, const __int64 &s )
{
	if ( IsPresent( nSide ) )
		GetDirEntry( nSide ).SetFileSize( s );
}

const CString CViewDirItem::GetFileSizeStr( int nSide ) const
{
	if ( IsDeleted() )
		return _T("del");
	CString strFS;
	if ( IsMarkDirty() )
		strFS = _T("###");
	else if ( IsMarkPending() )
		strFS = _T("???");
	else if ( GetFileSize(nSide) != 0 )
	{
		LPTSTR psz = strFS.GetBuffer( 34 );
		_i64tot_s( GetFileSize(nSide), psz, 34, 10 );
		size_t nLen = _tcslen(psz);
		int nIns = nLen - 3;
		while ( nIns > 0 ) {
			LPTSTR pszE = psz + nIns;
			LPTSTR pszO = psz + nLen;
			++nLen;
			LPTSTR pszN = psz + nLen;
			while ( pszN > pszE ) {
				*pszN-- = *pszO--;
			}
			*pszE = '.';
			nIns -= 3;
		}
		strFS.ReleaseBuffer();
	}
//	else if ( IsMarkPending() )
//		strFS = _T("???");
    
	return strFS;
}

void CViewDirItem::ActivateSide( int nSide, CDocDir *pDoc, DOCPOS pos )
{
	ASSERT(pos != NULL);
	ASSERT_KINDOF( CDocDir, pDoc );
	if ( IsValid(nSide) ) {
//		CDirEntry &de = GetDirEntry(nSide);
//		ASSERT( de.IsDel() );
//		de.SetViewItemPos(NULL);
		ResetDocRef(nSide);
	} else {
		ASSERT( s[nSide].ppd == NULL || s[nSide].ppd == pDoc );
		ASSERT( s[nSide].pos == NULL || s[nSide].pos == pos );
	}
	s[nSide].ppd = pDoc;
	s[nSide].pos = pos;
	if ( GetDirEntry(nSide).IsDir() )
		SetIcon( pDoc->GetSubIcon() );
}

void CViewDirItem::UpdateMark()
{
#ifdef _DEBUG
	if ( !IsDeleted() )
		m_strNameDebug.SetString( GetName() );
#endif

	SetMarkEqual();
	if ( s[left].pos == NULL || s[right].pos == NULL || IsDir() || 
			!IsPresent(left) || !IsPresent(right) )
	{
		SetMarkAll();
		return;
	}

	ASSERT( !IsDir() );

	if ( GetFileSize(0) == 0 && GetFileSize(1) == 0 )		// 20080708
		return;

	if ( GetDateTime(left) != GetDateTime(right) )
	{
		CTimeSpan dd = GetDateTime(right) - GetDateTime(left);
		LONGLONG ts = dd.GetTotalSeconds();
		if ( ts > 2 || ts < -2 ) {
			m_nMark &= maskLink | markAll;
			if ( GetDateTime(left) > GetDateTime(right) )
				m_nMark |= markTimeL;
			if ( GetDateTime(right) > GetDateTime(left) )
				m_nMark |= markTimeR;
		}
	}
	if ( GetFileSize(0) != GetFileSize(1) )
	{
		m_nMark &= maskLink | markAll;
		if ( GetFileSize(left) != 0 )
			m_nMark |= markSizeL;
		if ( GetFileSize(right) != 0 )
			m_nMark |= markSizeR;
	}
	m_nMark |= markDirty;
}

BOOL CViewDirItem::CompareCRC( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	ULONG crc0 = GetParentDoc(left)->ComputeCRC( GetParentPos(left), lpProgressRoutine, lpData );
	ULONG crc1 = GetParentDoc(right)->ComputeCRC( GetParentPos(right), lpProgressRoutine, lpData );
	return ( crc0 == crc1 && crc0 != 0 );
}

BOOL CViewDirItem::CompareCRCText( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	ULONG crc0 = GetParentDoc(left)->ComputeCRCText( GetParentPos(left), lpProgressRoutine, lpData );
	ULONG crc1 = GetParentDoc(right)->ComputeCRCText( GetParentPos(right), lpProgressRoutine, lpData );
	return ( crc0 == crc1 && crc0 != 0 );
}

int CViewDirItem::Compare( const CViewDirItem &d, int nSide, int nSortType ) const
{
//	TRACE2(" cmp %s - %s\n", GetName(), d.GetName() );

	int c;

	switch ( nSortType )
	{
	case ID_SORT_NAME:	// standard: dirs + files subordered by name ascending
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  -1 : 1 );
		break;

	case ID_SORT_MKS:	// MKS: files + dirs subordered by name ascending
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  1 : -1 );
		break;

	case ID_SORT_LEFT_SIZE: // files + dirs by size left
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  -1 : 1 );
		if ( GetFileSize(0) != d.GetFileSize(0) )
			return ( GetFileSize(0) > d.GetFileSize(0) ? 1 : -1 );
		break;

	case ID_SORT_LEFT_SIZE_DESC: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  1 : -1 );
		if ( GetFileSize(0) != d.GetFileSize(0) )
			return ( GetFileSize(0) > d.GetFileSize(0) ? -1 : 1 );
		break;

	case ID_SORT_LEFT_TIME: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  -1 : 1 );
		if ( IsPresent(0) && !d.IsPresent(0) )
			return 1;
		if ( !IsPresent(0) && d.IsPresent(0) )
			return -1;
		if ( IsPresent(0) && d.IsPresent(0) ) {
			if ( GetDateTime(0) != d.GetDateTime(0) )
				return ( GetDateTime(0) > d.GetDateTime(0) ? 1 : -1 );
		}
		break;

	case ID_SORT_LEFT_TIME_DESC: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  1 : -1 );
		if ( IsPresent(0) && !d.IsPresent(0) )
			return -1;
		if ( !IsPresent(0) && d.IsPresent(0) )
			return 1;
		if ( IsPresent(0) && d.IsPresent(0) ) {
			if ( GetDateTime(0) != d.GetDateTime(0) )
				return ( GetDateTime(0) > d.GetDateTime(0) ? -1 : 1 );
		}
		break;

	case ID_SORT_RIGHT_SIZE: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  -1 : 1 );
		if ( GetFileSize(1) != d.GetFileSize(1) )
			return ( GetFileSize(1) > d.GetFileSize(1) ? 1 : -1 );
		break;

	case ID_SORT_RIGHT_SIZE_DESC: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  1 : -1 );
		if ( GetFileSize(1) != d.GetFileSize(1) )
			return ( GetFileSize(1) > d.GetFileSize(1) ? -1 : 1 );
		break;

	case ID_SORT_RIGHT_TIME: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  -1 : 1 );
		if ( IsPresent(1) && !d.IsPresent(1) )
			return 1;
		if ( !IsPresent(1) && d.IsPresent(1) )
			return -1;
		if ( IsPresent(1) && d.IsPresent(1) ) {
			if ( GetDateTime(1) != d.GetDateTime(1) )
				return ( GetDateTime(1) > d.GetDateTime(1) ? 1 : -1 );
		}
		break;

	case ID_SORT_RIGHT_TIME_DESC: 
		if ( IsDir() != d.IsDir() )
			return ( IsDir() ?  1 : -1 );
		if ( IsPresent(1) && !d.IsPresent(1) )
			return -1;
		if ( !IsPresent(1) && d.IsPresent(1) )
			return 1;
		if ( IsPresent(1) && d.IsPresent(1) ) {
			if ( GetDateTime(1) != d.GetDateTime(1) )
				return ( GetDateTime(1) > d.GetDateTime(1) ? -1 : 1 );
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	c = GetName().CompareNoCase( d.GetName() );
	if ( c != 0 )
		return c;
	return GetName().Compare( d.GetName() );
}

int CViewDirItem::Compare( int nSide, const CDocDir *pDoc, POSITION pos ) const
{
	if ( pDoc == GetParentDoc(nSide) && 
		 pos  == GetParentPos(nSide) )
		return 0;

	// see CDocDir::CDirEntry::Compare

//	TRACE2(" v/d %s - %s\n", GetName(), pDoc->GetDirEntry( pos ).GetName() );

	int c;

	if ( IsDir() != pDoc->GetDirEntry( pos ).IsDir() )
		return ( IsDir() ?  -1 : 1 );

	c = GetName().CompareNoCase( pDoc->GetDirEntry( pos ).GetName() );
	if ( c != 0 )
		return c;
	return GetName().Compare( pDoc->GetDirEntry( pos ).GetName() );
}

void CViewDirItem::ResetDocRef( int nSide ) 
{ 
	if (s[nSide].pos != NULL) { 
		CDirEntry &de = GetDirEntry( nSide );
//		ASSERT( de.IsDel() );
		if ( de.GetDoc() != NULL )
			de.GetDoc()->Invalidate();
		de.SetViewItemPos(NULL); 
		s[nSide].pos = NULL;
	} 
}

void CViewDirItem::ClearItem( int nSide )
{
	if ( IsPresent(nSide) )
		GetDirEntry(nSide).SetDel();

	if ( IsValid(nSide) )
	{
		if ( GetDoc(nSide) != NULL )
			GetDoc(nSide)->ResetAll();
		ResetDocRef(nSide);
		UpdateMark();
	}
}

void CViewDirItem::FreeItem()
{
	ClearItem( 0 );
	ClearItem( 1 );
}

BOOL CViewDirItem::HandleOutdated( DWORD dwTicksNow )
{
//	TRACE1( " %d\n", dwTicksNow - m_dwTicksShow );
	if ( m_dwTicksShow == 0 || dwTicksNow < m_dwTicksShow + 5 * 60000 )
		return FALSE;

	m_dwTicksShow = 0;
	return TRUE;
}

void CViewDirItem::Invalidate()
{
	if ( !IsDir() )
		return;

	__int64 s0 = 0;

	if ( IsValid(0)  )
	{
		GetDirEntry(0).SetFileSize( s0 );
		if ( GetDoc(0) != NULL )
			GetDoc(0)->Invalidate();
	}
	if ( IsValid(1) )
	{
		GetDirEntry(1).SetFileSize( s0 );
		if ( GetDoc(1) != NULL )
			GetDoc(1)->Invalidate();
	}
	UpdateMark();
	SetMarkPending();
}

void CViewDirItem::DrawTreeItemSide(CDC* pDC, CDualTreeCtrl &tree, HTREEITEM hItem, LPNMTVCUSTOMDRAW lplvcd, int nSide)
{
	BOOL bDirPlus = tree.ItemHasChildren( hItem ) && (tree.GetItemState( hItem, TVIS_EXPANDED ) & TVIS_EXPANDED) == 0;	// 20070711
	bDirPlus |= IsMarkDirty();	// 20100820
	BOOL bSingle = (IsPresent(left) != IsPresent(right)) || IsMarkSingle(nSide);		// 20090107
	m_dwTicksShow = GetTickCount();		// 2012-01-31 mark item as shown

	// If this item is selected, set the background color 
	// and the text color to appropriate values. Also, erase
	// rect by filling it with the background color.
	if ( nSide == common )
	{
		bSingle |= IsMarkSingle();
		RECT rcNav = lplvcd->nmcd.rc;
		rcNav.right = rcNav.left;
		rcNav.left = 0;
		pDC->FillSolidRect(&rcNav, tree.GetWndColor() );
	}
	BOOL bHigh;
//	if ((lplvcd->nmcd.uItemState & CDIS_SELECTED) &&
	if (IsSel() &&
		(nSide == tree.GetSide() || nSide == common) )
	{
		bHigh = TRUE;
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		lplvcd->clrText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		lplvcd->clrTextBk = ::GetSysColor(COLOR_HIGHLIGHT);
		pDC->FillSolidRect(&lplvcd->nmcd.rc, 
			::GetSysColor(COLOR_HIGHLIGHT));
	}
	else if (bSingle && 				// (nSide!=left) || !IsPresent(left)))	// 20080625
		       (nSide == common || IsPresent(nSide)) )
	{
		bHigh = FALSE;
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		lplvcd->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
		pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetMarkSingleColor() );
		lplvcd->clrTextBk = tree.GetMarkSingleColor();
	}
	else
	{
		bHigh = FALSE;
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		lplvcd->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
		pDC->SetBkColor( tree.GetBkColor() );
		lplvcd->clrTextBk = tree.GetBkColor();
		if ( m_nMark != markEqual && (!IsDir() || bDirPlus || nSide == common) )
		{
			if ( nSide == left )
			{
				pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetMarkDirColor2() );
				lplvcd->clrTextBk = tree.GetMarkDirColor2();
			}
			else
			{
				if ( IsDir() && !bDirPlus && nSide == common ) {
					pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetMarkDirColor2() );
					lplvcd->clrTextBk = tree.GetMarkDirColor2();
				} else {
					pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetMarkDirColor() );
					lplvcd->clrTextBk = tree.GetMarkDirColor();
				}
			}
		} 
		else if ( nSide == left )
		{
			pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetMarkLiteColor());
		} 
		else
		{
			pDC->FillSolidRect(&lplvcd->nmcd.rc, tree.GetWndColor());
			lplvcd->clrTextBk = tree.GetWndColor();
		}
	}
	if ( nSide == common )
	{
		// drawing is done by control
	}
	else
	{
		// Draw datetime and size.
		if ( IsPresent( nSide ) )
		{
			COLORREF crTxt = pDC->GetTextColor();
			if ( !bHigh &&
				 (( nSide == left  && ((m_nMark & markTimeL) == markTimeL) ) || 
				  ( nSide == right && ((m_nMark & markTimeR) == markTimeR) ) ) )
			{
				pDC->SetTextColor( tree.GetTxtChangedColor() );
			}
			CRect rect = lplvcd->nmcd.rc;
			rect.left += 3;
			rect.right = rect.left + tree.GetCharWidth() * 17;	// 150
			CString strDT = GetDateTimeStr(nSide);
			pDC->DrawText(
				strDT,
				rect,
				DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

			if ( !bHigh &&
				 (( nSide == left  && ((m_nMark & markSizeL) == markSizeL) ) ||
				  ( nSide == right && ((m_nMark & markSizeR) == markSizeR) ) ) )
			{
				pDC->SetTextColor( tree.GetTxtChangedColor() );
			}
			else
			{
				pDC->SetTextColor( crTxt );
			}
			rect.left = rect.right;
			rect.right = lplvcd->nmcd.rc.right;
			CString strFS = GetFileSizeStr(nSide);
			if ( !strFS.IsEmpty() )
			{
//			if ( GetFileSize(nSide) != 0 )
//			{
//				CString strFS;
//				LPTSTR psz = strFS.GetBuffer( 34 );
//				_i64tot( GetFileSize(nSide), psz, 10 );
//				strFS.ReleaseBuffer();
				pDC->DrawText(
					strFS,
					&lplvcd->nmcd.rc,
					DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
			}
		}
	}

	if ((lplvcd->nmcd.uItemState & CDIS_FOCUS))
    {
        pDC->DrawFocusRect( &(lplvcd->nmcd.rc) );
    }
}

void CViewDirItem::OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo)
{
//	if ( pTVDispInfo->item.mask & TVIF_TEXT )
//	{
//		pTVDispInfo->item.pszText = _T("Hugo");
//	}
//	if ( pTVDispInfo->item.mask & TVIF_IMAGE )
//	{
//		;
//	}
//	if ( pTVDispInfo->item.mask & TVIF_SELECTEDIMAGE )
//	{
//		;
//	}
//	if ( pTVDispInfo->item.mask & TVIF_CHILDREN )
//	{
//		;
//	}
	ASSERT( FALSE );
	AfxThrowNotSupportedException( );	// should be implemented in derived class for treectrl
}
