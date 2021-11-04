#pragma once

#include "UndoTaskFileSync.h"
#include "ViewText.h"

class CViewText;
class CDocText;

class CUndoViewText :
	public CUndoTaskFileSync
{
public:
	CUndoViewText(CViewText *pView );
	virtual ~CUndoViewText(void);
	virtual BOOL DoFirst(void);
	virtual void SetModifiedFlag(int nChan, BOOL bFlag) { m_bDocModified[nChan] = bFlag; }
	virtual INT_PTR GetSize();

protected:
	BOOL SaveLines();
	BOOL SaveItemData( int nStart, int nCount );
	void UpdateView( CViewFileSync::Side nSide, int nLinesIns );
	void SetSelFromLineNo( CViewFileSync::Side nSide, int nLineNo, int nCount );
	void RestoreItemData();

	CViewText* GetView() const { return (CViewText *)m_pView; }
	CDocText* GetDoc( int s ) const { return GetView()->GetDoc(s); }

protected:
	typedef struct {
		int nLineNo[2];
		BOOL bMark;
	} ViewItem;
	CArray<ViewItem,ViewItem> m_aViewItems;
	int m_nLineNoStart[2];	// first LineNo in m_aViewItems
	int m_nLineNoEnd[2];	// first LineNo behind m_aViewItems
	int m_nStartSave;		// first item in m_aViewItems
	int m_nInsertedItems;	// LB items inserted by CompareUniqueBlock
	int m_nLineNoSel;
	int m_nLinesSel;
	int m_nStartInternalLineNo;
	int m_nLineNoSelOther;
	int m_nLinesSelOther;
	int m_nStartInternalLineNoOther;
	BOOL m_bDocModified[2];
	CStringArray m_astrRawLines;
};
