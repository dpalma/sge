/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "BitmapUtils.h"

#include "resmgr.h"
#include "imagedata.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ALIGN4BYTE(w) (((w) + 3) & ~3)

/////////////////////////////////////////////////////////////////////////////

bool LoadBitmap(const tChar * pszBitmap, HBITMAP * phBitmap)
{
   if (pszBitmap == NULL || phBitmap == NULL)
   {
      return false;
   }

   bool bResult = false;

   UseGlobal(ResourceManager);

   cImageData * pImageData = ImageLoad(pResourceManager, pszBitmap);

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

            for (int i = 0; i < pImageData->GetHeight(); i++)
            {
               memcpy(pDest, pSrc, srcScanLineSize);
               pDest += destScanLineSize;
               pSrc += srcScanLineSize;
            }

            HBITMAP hBitmap = CreateCompatibleBitmap(dc.GetSafeHdc(),
                                                     pImageData->GetWidth(),
                                                     pImageData->GetHeight());

            if (hBitmap != NULL)
            {
               BITMAPINFOHEADER bmInfo = {0};
               bmInfo.biSize = sizeof(BITMAPINFOHEADER);
               bmInfo.biWidth = pImageData->GetWidth();
               bmInfo.biHeight = pImageData->GetHeight();
               bmInfo.biPlanes = 1; 
               bmInfo.biBitCount = bitCount; 
               bmInfo.biCompression = BI_RGB;

               int nScanLines = SetDIBits(dc.GetSafeHdc(),
                                          hBitmap,
                                          0,
                                          pImageData->GetHeight(),
                                          pImageBits,
                                          (BITMAPINFO *)&bmInfo,
                                          DIB_RGB_COLORS);

               if (nScanLines > 0)
               {
                  *phBitmap = hBitmap;
                  bResult = true;
               }
            }

            delete [] pImageBits;
         }
      }

      delete pImageData;
   }
   else
   {
      HBITMAP hBitmap = (HBITMAP)LoadImage(NULL,
                                           pszBitmap,
                                           IMAGE_BITMAP,
                                           0, 0,
                                           LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

      if (hBitmap == NULL)
      {
         hBitmap = (HBITMAP)LoadImage(AfxGetResourceHandle(),
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
