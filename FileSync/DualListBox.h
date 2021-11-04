#pragma once

// CDualListBox

class CViewList;

//class CDualListBox : public CListBox
class CDualListBox : public CWnd
{
	DECLARE_DYNAMIC(CDualListBox)

public:
	CDualListBox();
	virtual ~CDualListBox();

	void PrepareFont( CFont *pFont ) { m_pFont = pFont; }
	int GetItemHeight() const { return m_nItemHeight; }
	void SetView( CViewList *pView ) { m_pView = pView; }
	int GetViewLineLen( int nResPixels ) const;
	int GetCharWidth() const { return m_nCharWidth; }
	void SetCharsLeft( int nChars ) { m_nCharsLeft = nChars; UpdOffs(); }
	void SetCharsRight( int nChars ) { m_nCharsRight = nChars; UpdOffs(); }
	int GetOffsLeft() const { return m_nOffsLeft; }
	int GetOffsRight() const { return m_nOffsRight; }
	void EnableDraw( BOOL b ) { m_bEnableDraw = b; }
	BOOL IsEnabledDraw() { return m_bEnableDraw; }
	void AddItems( int nItems );

	// CListBox methods to be simulated
	virtual BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	int InitStorage( int nItems, UINT nBytes );
	int AddString( LPCTSTR lpszItem );
	int InsertString( int index, LPCTSTR lpszItem );
	int DeleteString( UINT nIndex );
	void ResetContent( );
	int GetCount( ) const;
	int GetTopIndex( ) const { return GetScrollPos( SB_VERT ); }
	int GetAnchorIndex( ) const { return m_nAnchor; }
	int GetCaretIndex( ) const { return m_nCaret; }
	int GetSelCount( ) const;
	int SetSel( int nIndex, BOOL bSelect = TRUE );
	int SetTopIndex( int nIndex, BOOL bInvalidate = TRUE );
	void SetAnchorIndex( int nIndex );
	int SetCaretIndex( int nIndex, BOOL bScroll = TRUE );
	int SelItemRange( BOOL bSelect, int nFirstItem, int nLastItem );
	UINT ItemFromPoint( CPoint pt, BOOL& bOutside ) const;
	int GetItemRect( int nIndex, LPRECT lpRect ) const;
	int GetSelFirst() { return m_nSelFirst; }

protected:
	DECLARE_MESSAGE_MAP()

	CFont *m_pFont;
	int m_nItemHeight;
	int m_nCharWidth;
	CViewList *m_pView;
	int m_nPageSize;
	int m_nCharsLeft;
	int m_nCharsRight;
	int m_nOffsLeft;
	int m_nOffsRight;
	BOOL m_bEnableDraw;

	// CListBox simulation
	int m_nItems;
	int m_nAnchor;
	int m_nCaret;
	int m_nSelFirst;
	int m_nSelLast;

public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnLbnSelchange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	int GetBottomIndex(void);
	void UpdOffs();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};


