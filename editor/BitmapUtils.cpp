/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "BitmapUtils.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

#define ALIGN4BYTE(w) (((w) + 3) & ~3)

///////////////////////////////////////////////////////////////////////////////
// Use NTSC constants for converting color to gray

byte GrayLevel(COLORREF color)
{
   double r = GetRValue(color);
   double g = GetGValue(color);
   double b = GetBValue(color);

   double intensity =
      0.3 * (r / 255) +
      0.6 * (g / 255) +
      0.1 * (b / 255);

   if (intensity < 0)
   {
      intensity = 0;
   }
   else if (intensity > 1)
   {
      intensity = 1;
   }

   return (byte)(intensity * 255);
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
   if (!dcMem.CreateCompatibleDC(hDC))
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
   bmp.CreateCompatibleBitmap(hDC, cxImage, cyImage);
   ATLASSERT(bmp.m_hBitmap != NULL);

   // draw image into memory DC--fill BG white first
   HBITMAP hBmpOld = dcMem.SelectBitmap(bmp);
   dcMem.PatBlt(0, 0, cxImage, cyImage, WHITENESS);

   // If white is the text color, we can't use the normal painting since
   // it would blend with the WHITENESS, but the mask is OK
   UINT uDrawStyle = (::GetSysColor(COLOR_BTNTEXT) == RGB(255, 255, 255)) ? ILD_MASK : ILD_NORMAL;
   ImageList_Draw(hImageList, iImage, dcMem, x, y, uDrawStyle);
   CDCHandle(hDC).DitherBlt(x, y, cxImage, cyImage, dcMem, NULL, 0, 0, hBrushBackground, hBrush3DEffect, hBrushDisabledImage);
   dcMem.SelectBitmap(hBmpOld);

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

bool LoadBitmap(const cImageData * pImageData, HBITMAP * phBitmap)
{
   bool bResult = false;

   if (pImageData != NULL)
   {
      static const int bitCounts[] =
      {
         0, // kPF_Grayscale
         0, // kPF_ColorMapped
         16, // kPF_RGB555
         16, // kPF_BGR555
         16, // kPF_RGB565
         16, // kPF_BGR565
         16, // kPF_RGBA1555
         16, // kPF_BGRA1555
         24, // kPF_RGB888
         24, // kPF_BGR888
         32, // kPF_RGBA8888
         32, // kPF_BGRA8888
      };

      int bitCount = bitCounts[pImageData->GetPixelFormat()];

      if (bitCount > 0)
      {
         CWindowDC dc(NULL);

         uint bytesPerPixel = bitCount / 8;

         uint alignedWidth = ALIGN4BYTE(pImageData->GetWidth());

         size_t imageBitsSize = ((pImageData->GetWidth() * bytesPerPixel) + (alignedWidth - pImageData->GetWidth())) * pImageData->GetHeight();

         byte * pImageBits = new byte[imageBitsSize];

         if (pImageBits != NULL)
         {
            size_t srcScanLineSize = pImageData->GetWidth() * bytesPerPixel;
            size_t destScanLineSize = ((pImageData->GetWidth() * bytesPerPixel) + (alignedWidth - pImageData->GetWidth()));

            byte * pSrc = (byte *)pImageData->GetData();
            byte * pDest = pImageBits;

            for (uint i = 0; i < pImageData->GetHeight(); i++)
            {
               memcpy(pDest, pSrc, srcScanLineSize);
               pDest += destScanLineSize;
               pSrc += srcScanLineSize;
            }

            CBitmap bitmap;
            if (bitmap.CreateCompatibleBitmap(dc, pImageData->GetWidth(), pImageData->GetHeight()))
            {
               BITMAPINFOHEADER bmInfo = {0};
               bmInfo.biSize = sizeof(BITMAPINFOHEADER);
               bmInfo.biWidth = pImageData->GetWidth();
               bmInfo.biHeight = pImageData->GetHeight();
               bmInfo.biPlanes = 1; 
               bmInfo.biBitCount = bitCount; 
               bmInfo.biCompression = BI_RGB;

               int nScanLines = SetDIBits(dc,
                                          bitmap,
                                          0,
                                          pImageData->GetHeight(),
                                          pImageBits,
                                          (BITMAPINFO *)&bmInfo,
                                          DIB_RGB_COLORS);

               if (nScanLines > 0)
               {
                  *phBitmap = bitmap.Detach();
                  bResult = true;
               }
            }

            delete [] pImageBits;
         }
      }
   }

   return bResult;   
}

/////////////////////////////////////////////////////////////////////////////

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap)
{
   if (pszBitmap == NULL || phBitmap == NULL)
   {
      return false;
   }

   bool bResult = false;

   cAutoIPtr<IResource> pRes;
   UseGlobal(ResourceManager2);
   if (pResourceManager2->Load(tResKey(pszBitmap, kRC_Image), &pRes) == S_OK)
   {
      cImageData * pImageData = NULL;
      if (pRes->GetData((void**)&pImageData) == S_OK && pImageData != NULL)
      {
         bResult = LoadBitmap(pImageData, phBitmap);
         delete pImageData;
      }
   }

   if (!bResult)
   {
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
         bResult = true;
      }
   }

   return bResult;
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

/////////////////////////////////////////////////////////////////////////////
