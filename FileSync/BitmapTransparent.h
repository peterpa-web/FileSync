#pragma once
#include "afxwin.h"

class CBitmapTransparent :
	public CBitmap
{
public:
	CBitmapTransparent(void);
	~CBitmapTransparent(void);
	void DrawTransparent(CDC * pDC, int x, int y, COLORREF crMask = RGB(0, 128, 128), int nReduce=0);

protected:
	COLORREF	m_crBlack;
	COLORREF	m_crWhite;
};

