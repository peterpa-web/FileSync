#include "StdAfx.h"
#include "BitmapTransparent.h"

// see http://codeguru.earthweb.com/cpp/g-m/bitmap/print.php/c1753

CBitmapTransparent::CBitmapTransparent(void)
{
	m_crBlack = 0;
	m_crWhite = RGB(255,255,255);
}


CBitmapTransparent::~CBitmapTransparent(void)
{
}

void CBitmapTransparent::DrawTransparent(CDC * pDC, int x, int y, COLORREF crMask, int nReduce)
{
	COLORREF crOldBack = pDC->SetBkColor(m_crWhite);
	COLORREF crOldText = pDC->SetTextColor(m_crBlack);
	CDC dcImage, dcTrans;

	// Create two memory dcs for the image and the mask
	dcImage.CreateCompatibleDC(pDC);
	dcTrans.CreateCompatibleDC(pDC);

	// Select the image into the appropriate dc
	CBitmap* pOldBitmapImage = dcImage.SelectObject(this);

	// Create the mask bitmap
	CBitmap bitmapTrans;
	BITMAP bm;
	GetBitmap(&bm);
	int nWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;
	bitmapTrans.CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	// Select the mask bitmap into the appropriate dc
	CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

	// Build mask based on transparent colour
	dcImage.SetBkColor(crMask);
	dcTrans.BitBlt(0, 0, nWidth, nHeight, &dcImage, 0, 0, SRCCOPY);

	// Do the work - True Mask method - cool if not actual display
	nWidth -= 2 * nReduce;
	nHeight -= 2 * nReduce;
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, nReduce, nReduce, SRCINVERT);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcTrans, nReduce, nReduce, SRCAND);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, nReduce, nReduce, SRCINVERT);

	// Restore settings
	dcImage.SelectObject(pOldBitmapImage);
	dcTrans.SelectObject(pOldBitmapTrans);
	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);
}
