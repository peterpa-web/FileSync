#pragma once
#include "ViewList.h"
#include "DocText.h"

class CDocText;
class CEditLine;

class CViewText :
	public CViewList
{
	friend class CUndoViewText;

	DECLARE_DYNCREATE(CViewText)

public:
	CViewText(void);
	virtual ~CViewText(void);

	void CopyDocLines( int nSide, int nItem, int nCount, CStringArray &astrLinesRaw );
	void InsertDocLines( int nSide, int &nLineNoStart, const CStringArray &astrLinesRaw );
	void RemoveDocLines( int nSide, int nLineNoStart, int nCount );
	int GetDocLineCount( int nSide );
	virtual BOOL IsNoDifference();

protected:
	typedef struct {
		POSLINE pos[2];	// left,right position to LineData within document or NULL
		BOOL bMark;
	} ItemData;
	typedef CArray<ItemData,ItemData> CItemDataArray;
	CItemDataArray m_aItemData;
	int m_nWidthLineNo;
	int m_nCharOffs;
	CEditLine *m_pEdit;

	CDocText* GetDoc(int nSide) const { return (CDocText *)m_pDoc[nSide]; }
	CDocText* GetCurrDoc() const { return GetDoc(s_nSide); }

	void InsertItem( int nItem );
	void InsertItem( int nItem, const ItemData &d );
	void InsertItems( int nItem, CItemDataArray *paItems );
	void AddItem( const ItemData &d ) { InsertItem( (int)m_aItemData.GetSize(), d ); }
	void AddItems( CItemDataArray *paItems );
	void DeleteItems( int nItem, int nCount = 1 );
	void DrawLBItemSide(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct, int nSide);

	virtual void DeleteContents();
	void DeleteContents2();
	virtual BOOL CompareView();
	BOOL CompareUniqueBlock1( CItemDataArray &aItems,
					CDocText *pDocL, POSLINE posL, const POSLINE posLM, 
					CDocText *pDocR, POSLINE posR, const POSLINE posRM );
	void CompareUniqueBlock2( CItemDataArray &aItems,
					const CDocText *pDocL, POSLINE posL, CPosArray &aposL, int &nL, const POSLINE posLM, 
					const CDocText *pDocR, POSLINE posR, CPosArray &aposR, int &nR, const POSLINE posRM );
	int CompareBlock( CItemDataArray &aItems,
					  const CDocText *pDocL, POSLINE posL, POSLINE posLU, 
					  const CDocText *pDocR, POSLINE posR, POSLINE posRU );
	void InsertBlock( CItemDataArray &aItems,
					  const CDocText *pDocL, POSLINE &posL, const POSLINE posLU, int &nLinesL, 
					  const CDocText *pDocR, POSLINE &posR, const POSLINE posRU, int &nLinesR );
	int MatchBlock( CItemDataArray &aItems, BOOL bRight,
					  const CDocText *pDoc1, POSLINE &pos1, const POSLINE pos1U, int &nLines1, 
					  const CDocText *pDoc2, POSLINE &pos2, const POSLINE pos2U, int &nLines2 );
	void FindStartItem( int &nItem, POSLINE &posL, POSLINE &posR );
	void FindEndItem( int &nItem, POSLINE &posL, POSLINE &posR );
	void UpdateHScroll();
	BOOL IsAnySel();
	HGLOBAL SelCopy();
	void FindSelectedLines( int nSide, int nItem, int nItemLast, int &nLineNoSel, int &nLinesSel );
	void FindItemRange( int nSide, int nItem, int &nStart, int &nCount );
	POSLINE FindInternalLine( int nSide, int nItem );
	int GetInternalLineNo( int nSide, POSLINE pos );
	POSLINE InsertInternalLines( int nSide, POSLINE &pos, const CStringArray &astrLines );
	virtual void OnDeactivateApp();
	void QuickFix(UINT nItem, int nSide, const CRect &rect, CPoint point);
	void QuickFixBlock(UINT nItem, int nSide, const CRect &rect, CPoint point);
	void QuickFixLine(UINT nItem, int nSide, const CRect &rect, CPoint point);

public:
	virtual void DrawLBItem(CDC* pDC, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void OnListLButtonDown();
	virtual void OnListDblClk(UINT nItem, int nSide, const CRect &rect, CPoint point);
	virtual void OnListVScroll();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

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
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditInsertafter(CCmdUI *pCmdUI);
	afx_msg void OnEditInsertafter();
	afx_msg void OnUpdateEditInsertbefore(CCmdUI *pCmdUI);
	afx_msg void OnEditInsertbefore();
	afx_msg void OnUpdateEditReplacesel(CCmdUI *pCmdUI);
	afx_msg void OnEditReplacesel();
	afx_msg void OnEditPref();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDel(CCmdUI *pCmdUI);
	afx_msg void OnEditDel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditLineEnter();
	afx_msg void OnSearch();
	afx_msg void OnSearchB();
	afx_msg void OnSearchF();
	afx_msg void OnHelpContext();
	afx_msg void OnUpdateViewText(CCmdUI *pCmdUI);
};
