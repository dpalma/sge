/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_BITMAPUTILS_H)
#define INCLUDED_BITMAPUTILS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HBITMAP);

class cImageData;

///////////////////////////////////////////////////////////////////////////////

#define GetRValue16(color) ((uint16)(GetRValue((color))<<8))
#define GetGValue16(color) ((uint16)(GetGValue((color))<<8))
#define GetBValue16(color) ((uint16)(GetBValue((color))<<8))

/////////////////////////////////////////////////////////////////////////////

byte GrayLevel(COLORREF color);

HIMAGELIST ImageList_CreateGrayscale(HIMAGELIST hImageList);

bool LoadBitmap(const cImageData * pImageData, HBITMAP * phBitmap);

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap);

HBITMAP StretchCopyBitmap(uint width, uint height, HBITMAP hSrcBitmap, uint srcX, uint srcY, uint srcWidth, uint srcHeight);

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_BITMAPUTILS_H)
