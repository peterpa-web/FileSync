#pragma once
#include "ViewList.h"

class CDocHex;

class CViewHex :
	public CViewList
{
	DECLARE_DYNCREATE(CViewHex)

public:
	CViewHex(void);
	virtual ~CViewHex(void);

protected:
	typedef struct {
		int offs;
		int len;
	} CPos;
	typedef struct {
		CPos d[2];
		BOOL bMark;
	} ItemData;
	typedef CArray<ItemData,ItemData> CItemDataArray;
	CItemDataArray m_aItemData;
	int m_nWidthLineNo;
	int m_nCharOffs;
	int m_nItem;		// for Compare/AddItems
	int m_nMark;		// for Compare/AddItems

	CDocHex* GetDoc(int nSide) const { return (CDocHex *)m_pDoc[nSide]; }
	CDocHex* GetCurrDoc() const { return GetDoc(s_nSide); }

	void InsertItem( int nItem );
	void InsertItem( int nItem, const ItemData &d );
	void InsertItems( int nItem, CItemDataArray *paItems );
	void AddItem( const ItemData &d ) { InsertItem( (int)m_aItemData.GetSize(), d ); }
	void AddItems( CItemDataArray *paItems );
	void AddItems( int n, int nMax, int o, int oMax, BOOL bMark );
	void DeleteItems( int nItem, int nCount = 1 );
	void DrawLBItemSide(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct, int nSide);

	virtual void DeleteContents();
	void DeleteContents2();
	virtual BOOL CompareView();
	void UpdateHScroll();

public:
	virtual void DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
public:
	afx_msg void OnUpdateViewNextDiff(CCmdUI *pCmdUI);
	afx_msg void OnViewNextDiff();
	afx_msg void OnUpdateViewPrevDiff(CCmdUI *pCmdUI);
	afx_msg void OnViewPrevDiff();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSearch();
	afx_msg void OnSearchB();
	afx_msg void OnSearchF();
	afx_msg void OnHelpContext();
};
