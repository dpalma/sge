/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_BITMAPUTILS_H)
#define INCLUDED_BITMAPUTILS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HBITMAP);

class cImageData;

/////////////////////////////////////////////////////////////////////////////

bool LoadBitmap(const cImageData * pImageData, HBITMAP * phBitmap);

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap);

HBITMAP StretchCopyBitmap(uint width, uint height, HBITMAP hSrcBitmap, uint srcX, uint srcY, uint srcWidth, uint srcHeight);

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_BITMAPUTILS_H)
