/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_BITMAPUTILS_H)
#define INCLUDED_BITMAPUTILS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HBITMAP);

F_DECLARE_INTERFACE(ITerrainTileSet);

///////////////////////////////////////////////////////////////////////////////

#define GetRValue16(color) ((uint16)(GetRValue((color))<<8))
#define GetGValue16(color) ((uint16)(GetGValue((color))<<8))
#define GetBValue16(color) ((uint16)(GetBValue((color))<<8))

/////////////////////////////////////////////////////////////////////////////

// STDCALL to match the signature of ImageList_Draw exactly
BOOL STDCALL ImageList_DrawDisabled(HIMAGELIST hImageList, int iImage, HDC hDC, int x, int y, uint drawStyle);

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap);

HBITMAP StretchCopyBitmap(uint width, uint height, HBITMAP hSrcBitmap, uint srcX, uint srcY, uint srcWidth, uint srcHeight);

tResult TerrainTileSetCreateImageList(ITerrainTileSet * pTileSet, uint dimension, HIMAGELIST * phImageList);

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_BITMAPUTILS_H)
