/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "BitmapUtils.h"

#include "terrainapi.h"

#include "resourceapi.h"
#include "imageapi.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////

// Helper function for painting a disabled toolbar or menu bitmap
// This function can take either an HBITMAP (for SS) or a DC with 
//           the bitmap already painted (for cmdbar)
BOOL DitherBlt(HDC hDC, int x, int y, int nWidth, int nHeight, HDC hSrcDC, HBITMAP hBitmap, int xSrc, int ySrc,
		HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE),
		HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT),
		HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW))
{
	Assert(hDC != NULL || hBitmap != NULL);
	Assert(nWidth > 0 && nHeight > 0);
	
	// Create a generic DC for all BitBlts
	HDC hMemDC = (hSrcDC != NULL) ? hSrcDC : ::CreateCompatibleDC(hDC);
	if(hMemDC == NULL)
		return FALSE;
	
	// Create a DC for the monochrome DIB section
	CDC dcBW;
   dcBW.CreateCompatibleDC(CDC::FromHandle(hDC));
	Assert(dcBW.m_hDC != NULL);
	if(dcBW.m_hDC == NULL)
	{
		if(hSrcDC == NULL)
			DeleteDC(hMemDC);
		return FALSE;
	}

	// Create the monochrome DIB section with a black and white palette
	struct RGBBWBITMAPINFO
	{
		BITMAPINFOHEADER bmiHeader; 
		RGBQUAD bmiColors[2]; 
	};

	RGBBWBITMAPINFO rgbBWBitmapInfo = 
	{
		{ sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
		{ { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 } }
	};

	VOID* pbitsBW;
	CBitmap bmpBW;
   bmpBW.Attach(CreateDIBSection(dcBW, (LPBITMAPINFO)&rgbBWBitmapInfo, DIB_RGB_COLORS, &pbitsBW, NULL, 0));
	if(bmpBW.m_hObject == NULL)
	{
		if(hSrcDC == NULL)
			DeleteDC(hMemDC);
		return FALSE;
	}
	
	// Attach the monochrome DIB section and the bitmap to the DCs
	CBitmap * pbmOldBW = dcBW.SelectObject(&bmpBW);
	HGDIOBJ hbmOldMemDC = NULL;
	if(hBitmap != NULL)
		hbmOldMemDC = SelectObject(hMemDC, hBitmap);

	// Block: Dark gray removal: we want (128, 128, 128) pixels to become black and not white
	{
		CDC dcTemp1, dcTemp2;
      dcTemp1.CreateCompatibleDC(CDC::FromHandle(hDC));
		dcTemp2.CreateCompatibleDC(CDC::FromHandle(hDC));
		CBitmap bmpTemp1;
      bmpTemp1.CreateCompatibleBitmap(CDC::FromHandle(hMemDC), nWidth, nHeight);
		CBitmap bmpTemp2;
		bmpTemp2.CreateBitmap(nWidth, nHeight, 1, 1, NULL);
		CBitmap * pOldBmp1 = dcTemp1.SelectObject(&bmpTemp1);
		CBitmap * pOldBmp2 = dcTemp2.SelectObject(&bmpTemp2);
		// Let's copy our image, it will be altered
      dcTemp1.BitBlt(0, 0, nWidth, nHeight, CDC::FromHandle(hMemDC), xSrc, ySrc, SRCCOPY);

		// All dark gray pixels will become white, the others black
		dcTemp1.SetBkColor(RGB(128, 128, 128));
		dcTemp2.BitBlt(0, 0, nWidth, nHeight, &dcTemp1, 0, 0, SRCCOPY);
		// Do an XOR to set to black these white pixels
		dcTemp1.BitBlt(0, 0, nWidth, nHeight, &dcTemp2, 0, 0, SRCINVERT);

		// BitBlt the bitmap into the monochrome DIB section
		// The DIB section will do a true monochrome conversion
		// The magenta background being closer to white will become white
		dcBW.BitBlt(0, 0, nWidth, nHeight, &dcTemp1, 0, 0, SRCCOPY);

		// Cleanup
		dcTemp1.SelectObject(pOldBmp1);
		dcTemp2.SelectObject(pOldBmp2);
	}
	
	// Paint the destination rectangle using hBrushBackground
	if(hBrushBackground != NULL)
	{
		RECT rc = { x, y, x + nWidth, y + nHeight };
      ::FillRect(hDC, &rc, hBrushBackground);
	}

	// BitBlt the black bits in the monochrome bitmap into hBrush3DEffect color in the destination DC
	// The magic ROP comes from the Charles Petzold's book
	HGDIOBJ hOldBrush = SelectObject(hDC, hBrush3DEffect);
	BitBlt(hDC, x + 1, y + 1, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

	// BitBlt the black bits in the monochrome bitmap into hBrushDisabledImage color in the destination DC
	SelectObject(hDC, hBrushDisabledImage);
	BitBlt(hDC, x, y, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

	SelectObject(hDC, hOldBrush);
	dcBW.SelectObject(pbmOldBW);
	SelectObject(hMemDC, hbmOldMemDC);

	if(hSrcDC == NULL)
		DeleteDC(hMemDC);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// See CCommandBarCtrlImpl::DrawBitmapDisabled()

BOOL STDCALL ImageList_DrawDisabled(HIMAGELIST hImageList, int iImage,
                                    HDC hDC, int x, int y, uint drawStyle)
{
   HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE);
   HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT);
   HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW);

   // create memory DC
   CDC dcMem;
   if (!dcMem.CreateCompatibleDC(CDC::FromHandle(hDC)))
   {
      return FALSE;
   }

   IMAGEINFO imageInfo;
   if (!ImageList_GetImageInfo(hImageList, iImage, &imageInfo))
   {
      return FALSE;
   }

   int cxImage = imageInfo.rcImage.right - imageInfo.rcImage.left;
   int cyImage = imageInfo.rcImage.bottom - imageInfo.rcImage.top;

   // create mono or color bitmap
   CBitmap bmp;
   bmp.CreateCompatibleBitmap(CDC::FromHandle(hDC), cxImage, cyImage);
   Assert(bmp.m_hObject != NULL);

   // draw image into memory DC--fill BG white first
   CBitmap * pBmpOld = dcMem.SelectObject(&bmp);
   dcMem.PatBlt(0, 0, cxImage, cyImage, WHITENESS);

   // If white is the text color, we can't use the normal painting since
   // it would blend with the WHITENESS, but the mask is OK
   UINT uDrawStyle = (::GetSysColor(COLOR_BTNTEXT) == RGB(255, 255, 255)) ? ILD_MASK : ILD_NORMAL;
   ImageList_Draw(hImageList, iImage, dcMem, x, y, uDrawStyle);
   DitherBlt(hDC, x, y, cxImage, cyImage, dcMem, NULL, 0, 0, hBrushBackground, hBrush3DEffect, hBrushDisabledImage);
   dcMem.SelectObject(pBmpOld);

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap)
{
   if (pszBitmap == NULL || phBitmap == NULL)
   {
      return false;
   }

   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszBitmap, kRT_WindowsDDB, NULL, (void**)phBitmap) == S_OK)
   {
      return true;
   }

   HBITMAP hBitmap = (HBITMAP)LoadImage(NULL,
                                        pszBitmap,
                                        IMAGE_BITMAP,
                                        0, 0,
                                        LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

   if (hBitmap == NULL)
   {
      hBitmap = (HBITMAP)LoadImage(_Module.GetResourceInstance(),
                                   pszBitmap,
                                   IMAGE_BITMAP,
                                   0, 0,
                                   LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
   }

   if (hBitmap != NULL)       
   {
      *phBitmap = hBitmap;
      return true;
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////

// TODO: See Win32 API function CopyImage. Does the same thing
HBITMAP StretchCopyBitmap(uint width, uint height, HBITMAP hSrcBitmap,
                          uint srcX, uint srcY, uint srcWidth, uint srcHeight)
{
   if (width == 0 || height == 0 || hSrcBitmap == NULL || srcWidth == 0 || srcHeight == 0)
   {
      ErrorMsg("Invalid argument to StretchCopyBitmap\n");
      return NULL;
   }

   BOOL bSuccess = FALSE;

   HBITMAP hBitmap = NULL;

   HDC hScreenDC = GetDC(NULL);
   if (hScreenDC != NULL)
   {
      hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
      if (hBitmap != NULL)
      {
         HDC hSrcDC = CreateCompatibleDC(hScreenDC);
         if (hSrcDC != NULL)
         {
            HGDIOBJ hSrcOld = SelectObject(hSrcDC, hSrcBitmap);

            HDC hDestDC = CreateCompatibleDC(hScreenDC);
            if (hDestDC != NULL)
            {
               HGDIOBJ hDestOld = SelectObject(hDestDC, hBitmap);

               bSuccess = StretchBlt(hDestDC, 0, 0, width, height, hSrcDC,
                  srcX, srcY, srcWidth, srcHeight, SRCCOPY);

               DebugMsgIf1(!bSuccess, "Error %d in StretchBlt call\n", GetLastError());

               SelectObject(hDestDC, hDestOld);
               DeleteDC(hDestDC);
            }

            SelectObject(hSrcDC, hSrcOld);
            DeleteDC(hSrcDC);
         }
      }

      ReleaseDC(NULL, hScreenDC);
   }

   if (!bSuccess && hBitmap != NULL)
   {
      DeleteObject(hBitmap);
      hBitmap = NULL;
   }

   return hBitmap;
}

///////////////////////////////////////////////////////////////////////////////

tResult TerrainTileSetCreateImageList(ITerrainTileSet * pTileSet, uint dimension, HIMAGELIST * phImageList)
{
   if (pTileSet == NULL || phImageList == NULL)
   {
      return E_POINTER;
   }

   if (dimension == 0)
   {
      return E_INVALIDARG;
   }

   uint tileCount = 0;
   if (pTileSet->GetTileCount(&tileCount) != S_OK || tileCount == 0)
   {
      return E_FAIL;
   }

   HIMAGELIST hImageList = ImageList_Create(dimension, dimension, ILC_COLOR24, tileCount, 0);
   if (hImageList == NULL)
   {
      return E_FAIL;
   }

   UseGlobal(ResourceManager);

   for (uint i = 0; i < tileCount; i++)
   {
      cStr tileTexture;
      if (pTileSet->GetTileTexture(i, &tileTexture) == S_OK)
      {
         HBITMAP hbm = NULL;
         BITMAP bm = {0};
         if (pResourceManager->Load(tileTexture.c_str(), kRT_WindowsDDB, NULL, (void**)&hbm) == S_OK
            && GetObject(hbm, sizeof(bm), &bm))
         {
            HBITMAP hSizedBm = StretchCopyBitmap(dimension, dimension, hbm, 0, 0,
               min(dimension, (uint)bm.bmWidth), min(dimension, (uint)bm.bmHeight));
            if (hSizedBm != NULL)
            {
               Verify(ImageList_Add(hImageList, hSizedBm, NULL) >= 0);
               DeleteObject(hSizedBm);
            }
         }
      }
   }

   *phImageList = hImageList;
   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
