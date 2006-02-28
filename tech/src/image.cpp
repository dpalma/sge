///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "image.h"

#include "color.h"
#include "globalobj.h"
#include "resourceapi.h"

#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

uint BytesPerPixel(ePixelFormat pixelFormat)
{
   if (pixelFormat <= kPF_ERROR || pixelFormat >= kPF_NumPixelFormats)
   {
      return 0;
   }

   static const uint bytesPerPixel[] =
   {
      1, // kPF_Grayscale
      1, // kPF_ColorMapped
      2, // kPF_RGB555
      2, // kPF_BGR555
      2, // kPF_RGB565
      2, // kPF_BGR565
      2, // kPF_RGBA1555
      2, // kPF_BGRA1555
      3, // kPF_RGB888
      3, // kPF_BGR888
      4, // kPF_RGBA8888
      4, // kPF_BGRA8888
   };

   Assert(_countof(bytesPerPixel) == kPF_NumPixelFormats);

   return bytesPerPixel[pixelFormat];
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImage
//

///////////////////////////////////////

cImage::cImage(uint width, uint height, ePixelFormat pixelFormat, byte * pData)
 : m_width(width)
 , m_height(height)
 , m_pixelFormat(pixelFormat)
 , m_pData(pData)
{
}

///////////////////////////////////////

cImage::~cImage()
{
   // This is the memory allocated in ImageCreate() below
   delete [] m_pData;
   m_pData = NULL;
}

///////////////////////////////////////

uint cImage::GetWidth() const
{
   return m_width;
}

///////////////////////////////////////

uint cImage::GetHeight() const
{
   return m_height;
}

///////////////////////////////////////

ePixelFormat cImage::GetPixelFormat() const
{
   return m_pixelFormat;
}

///////////////////////////////////////

const void * cImage::GetData() const
{
   return m_pData;
}

///////////////////////////////////////

tResult cImage::GetPixel(uint x, uint y, cColor * pPixel) const
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   if (pPixel == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = BytesPerPixel(GetPixelFormat());
   byte * pImagePixel = m_pData + (y * GetWidth() * bytesPerPixel) + (x * bytesPerPixel);

   switch (GetPixelFormat())
   {
      case kPF_RGB888:
      {
         *pPixel = cColor(
            static_cast<float>(pImagePixel[0]) / 255.0f,
            static_cast<float>(pImagePixel[1]) / 255.0f,
            static_cast<float>(pImagePixel[2]) / 255.0f);
         return S_OK;
      }

      case kPF_BGR888:
      {
         *pPixel = cColor(
            static_cast<float>(pImagePixel[2]) / 255.0f,
            static_cast<float>(pImagePixel[1]) / 255.0f,
            static_cast<float>(pImagePixel[0]) / 255.0f);
         return S_OK;
      }

      case kPF_RGBA8888:
      {
         *pPixel = cColor(
            static_cast<float>(pImagePixel[0]) / 255.0f,
            static_cast<float>(pImagePixel[1]) / 255.0f,
            static_cast<float>(pImagePixel[2]) / 255.0f,
            static_cast<float>(pImagePixel[3]) / 255.0f);
         return S_OK;
      }

      case kPF_BGRA8888:
      {
         *pPixel = cColor(
            static_cast<float>(pImagePixel[2]) / 255.0f,
            static_cast<float>(pImagePixel[1]) / 255.0f,
            static_cast<float>(pImagePixel[0]) / 255.0f,
            static_cast<float>(pImagePixel[3]) / 255.0f);
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cImage::SetPixel(uint x, uint y, const cColor & pixel)
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   uint bytesPerPixel = BytesPerPixel(GetPixelFormat());
   byte * pImagePixel = m_pData + ((y * GetWidth()) + x) * bytesPerPixel;

   byte r = static_cast<byte>(pixel.GetRed() * 255);
   byte g = static_cast<byte>(pixel.GetGreen() * 255);
   byte b = static_cast<byte>(pixel.GetBlue() * 255);
   byte a = static_cast<byte>(pixel.GetAlpha() * 255);

   switch (GetPixelFormat())
   {
      case kPF_RGB888:
      {
         pImagePixel[0] = r;
         pImagePixel[1] = g;
         pImagePixel[2] = b;
         return S_OK;
      }

      case kPF_BGR888:
      {
         pImagePixel[0] = b;
         pImagePixel[1] = g;
         pImagePixel[2] = r;
         return S_OK;
      }

      case kPF_RGBA8888:
      {
         pImagePixel[0] = r;
         pImagePixel[1] = g;
         pImagePixel[2] = b;
         pImagePixel[3] = a;
         return S_OK;
      }

      case kPF_BGRA8888:
      {
         pImagePixel[0] = b;
         pImagePixel[1] = g;
         pImagePixel[2] = r;
         pImagePixel[3] = a;
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cImage::GetSubImage(uint x, uint y, uint width, uint height, IImage * * ppSubImage) const
{
   if (width == 0 || height == 0)
   {
      return E_INVALIDARG;
   }

   if (ppSubImage == NULL)
   {
      return E_POINTER;
   }

   uint memSize = BytesPerPixel(GetPixelFormat()) * width * height;
   if (memSize == 0)
   {
      WarnMsg1("Invalid pixel format %d\n", GetPixelFormat());
      return E_FAIL;
   }

   byte * pImageData = new byte[memSize];
   if (pImageData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   uint scanLine = BytesPerPixel(GetPixelFormat()) * width;
   uint scanLine2 = BytesPerPixel(GetPixelFormat()) * GetWidth();

   byte * p = pImageData;
   const byte * p2 = static_cast<const byte *>(GetData()) + (scanLine2 * y) + x;

   for (uint i = 0; i < height; i++)
   {
      memcpy(p, p2, scanLine);
      p += scanLine;
      p2 += scanLine2;
   }

   cAutoIPtr<cImage> pSubImage(new cImage(width, height, GetPixelFormat(), pImageData));
   if (!pSubImage)
   {
      return E_OUTOFMEMORY;
   }

   *ppSubImage = CTAddRef(static_cast<IImage*>(pSubImage));
   return S_OK;
}

///////////////////////////////////////

tResult cImage::Clone(IImage * * ppImage)
{
   return ImageCreate(GetWidth(), GetHeight(), GetPixelFormat(), GetData(), ppImage);
}


///////////////////////////////////////////////////////////////////////////////
// Windows DDB resource type

#ifdef _WIN32

///////////////////////////////////////

void * WindowsDDBFromImage(void * pData, int dataLength, void * param)
{
   if (pData == NULL)
   {
      return NULL;
   }

   IImage * pImage = reinterpret_cast<IImage*>(pData);

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

   int bitCount = bitCounts[pImage->GetPixelFormat()];
   if (bitCount <= 0)
   {
      return NULL;
   }

   HBITMAP hBitmap = NULL;

   uint bytesPerPixel = bitCount / 8;
   uint alignedWidth = (pImage->GetWidth() + 3) & ~3;
   size_t imageMemSize = ((pImage->GetWidth() * bytesPerPixel) + (alignedWidth - pImage->GetWidth())) * pImage->GetHeight();

   byte * pImageBits = new byte[imageMemSize];
   if (pImageBits != NULL)
   {
      size_t srcScanLineSize = pImage->GetWidth() * bytesPerPixel;
      size_t destScanLineSize = ((pImage->GetWidth() * bytesPerPixel) + (alignedWidth - pImage->GetWidth()));

      byte * pSrc = (byte *)pImage->GetData();
      byte * pDest = pImageBits;

      for (uint i = 0; i < pImage->GetHeight(); i++)
      {
         memcpy(pDest, pSrc, srcScanLineSize);
         pDest += destScanLineSize;
         pSrc += srcScanLineSize;
      }

      HDC hWindowDC = GetWindowDC(NULL);
      if (hWindowDC != NULL)
      {
         // Creating compatible with window DC makes this a device-dependent bitmap
         hBitmap = CreateCompatibleBitmap(hWindowDC, pImage->GetWidth(), pImage->GetHeight());
         if (hBitmap != NULL)
         {
            BITMAPINFOHEADER bmInfo = {0};
            bmInfo.biSize = sizeof(BITMAPINFOHEADER);
            bmInfo.biWidth = pImage->GetWidth();
            bmInfo.biHeight = pImage->GetHeight();
            bmInfo.biPlanes = 1; 
            bmInfo.biBitCount = bitCount; 
            bmInfo.biCompression = BI_RGB;

            int nScanLines = SetDIBits(hWindowDC, hBitmap, 0, pImage->GetHeight(),
                                       pImageBits, (BITMAPINFO *)&bmInfo, DIB_RGB_COLORS);
            if (nScanLines <= 0)
            {
               DeleteObject(hBitmap);
               hBitmap = NULL;
            }
         }

         ReleaseDC(NULL, hWindowDC), hWindowDC = NULL;
      }

      delete [] pImageBits;
   }

   return hBitmap;
}

///////////////////////////////////////

void WindowsDDBUnload(void * pData)
{
   HBITMAP hBitmap = reinterpret_cast<HBITMAP>(pData);
   if ((hBitmap != NULL) && (GetObjectType(hBitmap) == OBJ_BITMAP))
   {
      DeleteObject(hBitmap);
   }
}

#endif


///////////////////////////////////////////////////////////////////////////////

extern void * TargaLoad(IReader * pReader);
extern void * BmpLoad(IReader * pReader);

///////////////////////////////////////

void ImageUnload(void * pData)
{
   reinterpret_cast<IImage*>(pData)->Release();
}

///////////////////////////////////////

tResult ImageRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_Image, _T("tga"), TargaLoad, NULL, ImageUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Image, _T("bmp"), BmpLoad, NULL, ImageUnload) == S_OK)
      {
#ifdef _WIN32
         if (pResourceManager->RegisterFormat(kRT_WindowsDDB, kRT_Image, NULL, NULL, WindowsDDBFromImage, WindowsDDBUnload) != S_OK)
         {
            return E_FAIL;
         }
#endif
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult ImageCreate(uint width, uint height, ePixelFormat pixelFormat, const void * pData, IImage * * ppImage)
{
   if (width == 0 || height == 0)
   {
      return E_INVALIDARG;
   }

   if (pixelFormat <= kPF_ERROR || pixelFormat >= kPF_NumPixelFormats)
   {
      return E_INVALIDARG;
   }

   if (ppImage == NULL)
   {
      return E_POINTER;
   }

   uint memSize = BytesPerPixel(pixelFormat) * width * height;
   if (memSize == 0)
   {
      WarnMsg1("Invalid pixel format %d\n", pixelFormat);
      return E_FAIL;
   }

   byte * pImageData = new byte[memSize];
   if (pImageData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (pData != NULL)
   {
      memcpy(pImageData, pData, memSize);
   }
   else
   {
      memset(pImageData, 0, memSize);
   }

   cAutoIPtr<cImage> pImage(new cImage(width, height, pixelFormat, pImageData));
   if (!pImage)
   {
      return E_OUTOFMEMORY;
   }

   *ppImage = CTAddRef(static_cast<IImage*>(pImage));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
