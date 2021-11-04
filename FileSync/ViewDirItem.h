#pragma once
#include "DualTreeItem.h"
#include "DualTreeData.h"
#include "DocDir.h"

class CDirEntry;

class CDualTreeCtrl;

class CViewDirItem :
	public CDualTreeItem
{
protected:
	enum Mark {			
		markNull    =     0,
		markSizeR   =     1,
		markSizeL   =     2,
		markTimeR   =     4,
		markTimeL   =     8,
		markAll     = 0x00f,	// markSizeR ... markTimeL
		markEqual   = 0x010,
		markDiff    = 0x020,	// don't compare binary
		markSingleR = 0x040,	// 20090107
		markSingleL = 0x080,
		markPending = 0x100, // 20090129 ? incomplete data, wait for expansion
		maskAll     = 0x1ff, // 20090128
		markDirty   = 0x200,  // 20090128 # to be checked
		maskLink    =0x3000,
		markLink1   =0x1000,  // 20120313
		markLink2   =0x2000  // 20120313
	};

	int m_nMark;
	struct _s {
		CDocDir* ppd;	// parent
		DOCPOS pos;
	} s[2];
	// new navigation 20100518
	HTREEITEM m_hItem;			// real tree item
	TREEPOS m_posParent;		// to identify sibling items
	TREEPOS m_posPrevSibling;
	TREEPOS m_posNextSibling;
	TREEPOS m_posLastChild;
	DWORD m_dwTicksShow;			// timestamp last view
	static CString s_strDel;

#ifdef _DEBUG
	CString m_strNameDebug;
#endif

public:
	CViewDirItem();
	CViewDirItem( TREEPOS posParent, CDocDir* ppdLeft, CDocDir* ppdRight );
	virtual ~CViewDirItem();
	const CString & GetName() const;
#ifdef _DEBUG
	const CString & GetNameDebug();
#endif
	CString GetFullPath() const; // (for debug)
	void GetDispName( LPWSTR pDest, int nSize ) const;
	int GetMark() const { return m_nMark; }
	void SetMarkEqual() { m_nMark &= maskLink; m_nMark |= markEqual; }
	void ResetMarkEqual() { m_nMark &= ~markEqual; }
	void SetMarkAll() { m_nMark &= maskLink; m_nMark |= markAll | markDirty; }
	void SetMarkDiff() { m_nMark &= maskLink | markAll; m_nMark |= markDiff; }
	void SetMarkSingle( int nSide) { m_nMark = ( nSide == 0 ? markSingleL : markSingleR ) ; }
	void ResetMarkSingle() { m_nMark &= ~(markSingleL | markSingleR); }
	void SetMarkDirty() { m_nMark |= markDirty; }
	void ResetMarkDirty() { m_nMark &= ~markDirty; }
	void SetMarkPending() { m_nMark &= maskLink | maskAll; m_nMark |= markPending; }
	void ResetMarkPending() { m_nMark &= ~markPending; }
	void SetMarkLink1() { m_nMark |= markLink1; }
	void SetMarkLink2() { m_nMark |= markLink2; }
	const BOOL IsMarkEqual() const { return (m_nMark & markEqual) != 0; }
	const BOOL IsMarkEqualorSingle() const { return (IsMarkEqual() || IsSingle() ); }
	const BOOL IsMarkDiff() const { return (m_nMark & markDiff) != 0; }
	const BOOL IsSingleBase() const { return ( IsPresent(0) != IsPresent(1) ); }
	const BOOL IsSingle() const { return (IsMarkSingle() || IsSingleBase() ); }
	const BOOL IsMarkSingle() const { return (m_nMark & markSingleR) != 0 || (m_nMark & markSingleL) != 0; }
	const BOOL IsMarkSingle( int nSide) const { return ( nSide == 0 ? (m_nMark & markSingleL) != 0 : (m_nMark & markSingleR) != 0 ); }
	const BOOL IsMarkDirty() const { return (m_nMark & markDirty) != 0; }
	const BOOL IsMarkPending() const { return (m_nMark & markPending) != 0; }
	const BOOL IsMarkLink() const { return (m_nMark & maskLink) != 0; }
	const BOOL IsMarkLink1() const { return (m_nMark & markLink1) != 0; }
//	const BOOL IsMarkLink2() const { return (m_nMark & markLink2) != 0; }
	const BOOL IsDeleted() const { return (s[0].pos == NULL && s[1].pos == NULL); }
	const BOOL IsValid( int nSide ) const {	return (s[nSide].pos != NULL); }
	const BOOL IsPresent( int nSide ) const;
	const BOOL IsDiff( int nSide ) const { return IsPresent(nSide) && (!IsMarkEqual() || !IsPresent(1-nSide)); }
	const BOOL IsDir() const;			// one CDirEntry IsDir
	const BOOL GetSide() const { return ( IsPresent(0) | (IsPresent(1) << 1) ); }
	const BOOL HasDirIcon() const;
	const BOOL HasTextIcon() const;
	const BOOL HasArcIcon() const;
	const BOOL HasDirOrArcIcon() const;		// has IconDir or IconZipRoot or IconIsoRoot
	const BOOL HasUnknownIcon() const;
	const int GetSideMarked() const;
	const CDirEntry & GetDirEntry( int nSide ) const { ASSERT( s[nSide].pos != NULL ); return s[nSide].ppd->GetDirEntry( s[nSide].pos ); }
	CDirEntry & GetDirEntry( int nSide ) { ASSERT( s[nSide].pos != NULL ); return s[nSide].ppd->GetDirEntry( s[nSide].pos ); }
	const CTime & GetDateTime( int nSide ) const;
	CString GetDateTimeStr( int nSide ) const;
//	const __int64 & GetFileSize( int nSide ) const;
	__int64 GetFileSize( int nSide ) const;
	void SetFileSize( int nSide, const __int64 &s );
	const CString GetFileSizeStr( int nSide ) const;
	CDocDir* GetParentDoc( int nSide ) const { return s[nSide].ppd; }
	DOCPOS GetParentPos( int nSide ) const { return s[nSide].pos; }
	CDocDir* GetDoc( int nSide ) const { return IsValid(nSide) ? GetDirEntry(nSide).GetDoc() : NULL; }
	void SetDoc( int nSide, CDocDir *p ) { GetDirEntry( nSide ).SetDoc( p ); }
	void SetParentDoc( int nSide, CDocDir *pDoc ) { s[nSide].ppd = pDoc; }
	void SetParentPos( int nSide, DOCPOS pos ) { ASSERT(s[nSide].ppd != NULL); s[nSide].pos = pos; }
	void ActivateSide( int nSide, CDocDir *pDoc, POSITION pos );
	void UpdateMark();
	BOOL CompareCRC( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	BOOL CompareCRCText( LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData );
	int Compare( const CViewDirItem &d, int nSide, int nSortType ) const;
	int Compare( int nSide, const CDocDir *pDoc, POSITION pos ) const;
	virtual void DrawTreeItemSide(CDC* pDC, CDualTreeCtrl &tree, HTREEITEM hItem, LPNMTVCUSTOMDRAW lplvcd, int nSide);
	virtual void OnTvnGetdispinfo(LPNMTVDISPINFO pTVDispInfo);
	HTREEITEM GetItemHandle() const { return m_hItem; }
	void SetItemHandle(HTREEITEM hItem) { m_hItem = hItem; }
	TREEPOS GetItemParentPos() const { return m_posParent; };
	void ResetDocRef( int nSide );
	void ClearItem( int nSide );
	void FreeItem();
	void Invalidate();
	void SetPrevSiblingPos( TREEPOS pos ) { m_posPrevSibling = pos; }
	void SetNextSiblingPos( TREEPOS pos ) { m_posNextSibling = pos; }
	void SetLastChildPos( POSITION pos ) { m_posLastChild = pos; }
	TREEPOS GetPrevSiblingPos() const { return m_posPrevSibling; }
	TREEPOS GetNextSiblingPos() const { return m_posNextSibling; }
	TREEPOS GetLastChildPos() const { return m_posLastChild; }
	void SetCurrentTicks( DWORD dwTicks ) { m_dwTicksShow = dwTicks; }
	DWORD GetCurrentTicks() const { return m_dwTicksShow; }
	BOOL HandleOutdated( DWORD dwTicksNow );
};
