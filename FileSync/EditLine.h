#pragma once


// CEditLine

class CEditLine : public CEdit
{
	DECLARE_DYNAMIC(CEditLine)

public:
	CEditLine();
	virtual ~CEditLine();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCharDelete();
};


