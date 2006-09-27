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

static const uint g_pixelFormatBytesPerPixel[] =
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

AssertAtCompileTime(_countof(g_pixelFormatBytesPerPixel) == kPF_NumPixelFormats);

uint BytesPerPixel(ePixelFormat pixelFormat)
{
   if (pixelFormat <= kPF_ERROR || pixelFormat >= kPF_NumPixelFormats)
   {
      return 0;
   }

   return g_pixelFormatBytesPerPixel[pixelFormat];
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

   byte rgba[4] = {0};

   tResult result = GetPixel(x, y, rgba);

   if (result == S_OK)
   {
      *pPixel = cColor(
         static_cast<float>(rgba[0]) / 255.0f,
         static_cast<float>(rgba[1]) / 255.0f,
         static_cast<float>(rgba[2]) / 255.0f,
         static_cast<float>(rgba[3]) / 255.0f);
   }

   return result;
}

///////////////////////////////////////

tResult cImage::SetPixel(uint x, uint y, const cColor & pixel)
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   byte rgba[4];
   rgba[0] = static_cast<byte>(pixel.GetRed() * 255.0f);
   rgba[1] = static_cast<byte>(pixel.GetGreen() * 255.0f);
   rgba[2] = static_cast<byte>(pixel.GetBlue() * 255.0f);
   rgba[3] = static_cast<byte>(pixel.GetAlpha() * 255.0f);

   return SetPixel(x, y, rgba);
}

///////////////////////////////////////

tResult cImage::GetPixel(uint x, uint y, byte rgba[4]) const
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   if (rgba == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = BytesPerPixel(GetPixelFormat());
   const byte * pImagePixel = m_pData + (y * GetWidth() * bytesPerPixel) + (x * bytesPerPixel);

   switch (GetPixelFormat())
   {
      case kPF_RGB565:
      {
         uint16 pixel = *(uint16 *)pImagePixel;
         rgba[0] = ((pixel >> 11) & 31) << 3;
         rgba[1] = ((pixel >> 5) & 63) << 2;
         rgba[2] = (pixel & 31) << 3;
         return S_OK;
      }

      case kPF_RGB888:
      {
         rgba[0] = pImagePixel[0];
         rgba[1] = pImagePixel[1];
         rgba[2] = pImagePixel[2];
         return S_OK;
      }

      case kPF_BGR888:
      {
         rgba[0] = pImagePixel[2];
         rgba[1] = pImagePixel[1];
         rgba[2] = pImagePixel[0];
         return S_OK;
      }

      case kPF_RGBA8888:
      {
         rgba[0] = pImagePixel[0];
         rgba[1] = pImagePixel[1];
         rgba[2] = pImagePixel[2];
         rgba[3] = pImagePixel[3];
         return S_OK;
      }

      case kPF_BGRA8888:
      {
         rgba[0] = pImagePixel[2];
         rgba[1] = pImagePixel[1];
         rgba[2] = pImagePixel[0];
         rgba[3] = pImagePixel[3];
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cImage::SetPixel(uint x, uint y, const byte rgba[4])
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   if (rgba == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = BytesPerPixel(GetPixelFormat());
   byte * pImagePixel = m_pData + ((y * GetWidth()) + x) * bytesPerPixel;

   switch (GetPixelFormat())
   {
      case kPF_RGB565:
      {
         uint16 * pPixel = (uint16 *)pImagePixel;
         *pPixel = (((uint16)rgba[0] >> 3) << 11) | (((uint16)rgba[1] >> 2) << 5)  | ((uint16)rgba[2] >> 3);
         return S_OK;
      }

      case kPF_RGB888:
      {
         pImagePixel[0] = rgba[0];
         pImagePixel[1] = rgba[1];
         pImagePixel[2] = rgba[2];
         return S_OK;
      }

      case kPF_BGR888:
      {
         pImagePixel[0] = rgba[2];
         pImagePixel[1] = rgba[1];
         pImagePixel[2] = rgba[0];
         return S_OK;
      }

      case kPF_RGBA8888:
      {
         pImagePixel[0] = rgba[0];
         pImagePixel[1] = rgba[1];
         pImagePixel[2] = rgba[2];
         pImagePixel[3] = rgba[3];
         return S_OK;
      }

      case kPF_BGRA8888:
      {
         pImagePixel[0] = rgba[2];
         pImagePixel[1] = rgba[1];
         pImagePixel[2] = rgba[0];
         pImagePixel[3] = rgba[3];
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

   HBITMAP hBitmap = NULL;
   if (ImageToWindowsBitmap(pImage, &hBitmap) == S_OK)
   {
      return hBitmap;
   }

   return NULL;
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

///////////////////////////////////////////////////////////////////////////////

static const int g_pixelFormatBitCounts[] =
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

AssertAtCompileTime(_countof(g_pixelFormatBitCounts) == kPF_NumPixelFormats);

TECH_API tResult ImageToWindowsBitmap(IImage * pImage, HBITMAP * phBitmap)
{
   if (pImage == NULL || phBitmap == NULL)
   {
      return E_POINTER;
   }

   ePixelFormat pixelFormat = pImage->GetPixelFormat();

   if (pixelFormat != kPF_RGB565
      && pixelFormat != kPF_BGR888
      && pixelFormat != kPF_BGRA8888
      && pixelFormat != kPF_RGB888
      && pixelFormat != kPF_RGBA8888)
   {
      ErrorMsg1("Cannot convert pixel format %d to Windows bitmap\n", pixelFormat);
      return E_INVALIDARG;
   }

   int bitCount = g_pixelFormatBitCounts[pixelFormat];
   if (bitCount <= 0)
   {
      return NULL;
   }

   HBITMAP hBitmap = NULL;

   uint bytesPerPixel = bitCount / 8;
   uint alignedWidth = (pImage->GetWidth() + 3) & ~3;
   size_t imageMemSize = ((pImage->GetWidth() * bytesPerPixel) + (alignedWidth - pImage->GetWidth())) * pImage->GetHeight();

   byte * pImageBits = new byte[imageMemSize];
   if (pImageBits == NULL)
   {
      return E_OUTOFMEMORY;
   }

   size_t srcScanLineSize = pImage->GetWidth() * bytesPerPixel;
   size_t destScanLineSize = ((pImage->GetWidth() * bytesPerPixel) + (alignedWidth - pImage->GetWidth()));

   const byte * pSrc = (const byte *)pImage->GetData();
   byte * pDest = pImageBits;

   for (uint i = 0; i < pImage->GetHeight(); i++)
   {
      if (pixelFormat == kPF_RGB565)
      {
         memcpy(pDest, pSrc, srcScanLineSize);
      }
      else if (pixelFormat == kPF_BGR888 || pixelFormat == kPF_BGRA8888)
      {
         memcpy(pDest, pSrc, srcScanLineSize);
      }
      else if (pixelFormat == kPF_RGB888 || pixelFormat == kPF_RGBA8888)
      {
         for (uint j = 0; j < pImage->GetWidth(); j++)
         {
            const byte * pSrcPixel = pSrc + (j * bytesPerPixel);
            byte * pDestPixel = pDest + (j * bytesPerPixel);
            if (bytesPerPixel == 3)
            {
               const byte bgr[3] = { pSrcPixel[2], pSrcPixel[1], pSrcPixel[0] };
               memcpy(pDestPixel, bgr, bytesPerPixel);
            }
            else if (bytesPerPixel == 4)
            {
               const byte bgra[4] = { pSrcPixel[2], pSrcPixel[1], pSrcPixel[0], pSrcPixel[3] };
               memcpy(pDestPixel, bgra, bytesPerPixel);
            }
         }
      }
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
         byte bmInfoBytes[sizeof(BITMAPINFOHEADER) + (3 * sizeof(DWORD))];
         memset(bmInfoBytes, 0, sizeof(bmInfoBytes));

         BITMAPINFOHEADER * pBmih = reinterpret_cast<BITMAPINFOHEADER *>(bmInfoBytes);

         pBmih->biSize = sizeof(BITMAPINFOHEADER);
         pBmih->biWidth = pImage->GetWidth();
         // The height is negative because IImage objects are top-down DIBs
         // (i.e., the origin is at the top left)
         pBmih->biHeight = -static_cast<int>(pImage->GetHeight());
         pBmih->biPlanes = 1; 
         pBmih->biBitCount = bitCount; 
         pBmih->biCompression = BI_RGB;

         if (pixelFormat == kPF_RGB565)
         {
            pBmih->biCompression = BI_BITFIELDS;

            DWORD * pBitFields = reinterpret_cast<DWORD *>(bmInfoBytes + sizeof(BITMAPINFOHEADER));

            pBitFields[0] = 0xF800;
            pBitFields[1] = 0x07E0;
            pBitFields[2] = 0x001F;
         }

         int nScanLines = SetDIBits(hWindowDC, hBitmap, 0, pImage->GetHeight(),
                                    pImageBits, reinterpret_cast<BITMAPINFO *>(pBmih), DIB_RGB_COLORS);
         if (nScanLines <= 0)
         {
            DeleteObject(hBitmap);
            hBitmap = NULL;
         }
      }

      ReleaseDC(NULL, hWindowDC), hWindowDC = NULL;
   }

   delete [] pImageBits;

   *phBitmap = hBitmap;

   return (hBitmap != NULL) ? S_OK : E_FAIL;
}

#endif // _WIN32


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
      delete [] pImageData;
      return E_OUTOFMEMORY;
   }

   *ppImage = CTAddRef(static_cast<IImage*>(pImage));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

void ImageApplyGamma(IImage * pImage, uint x, uint y, uint w, uint h, float gamma)
{
   if (pImage == NULL)
   {
      return;
   }

   if (gamma < 0.2f || gamma > 5.0f)
   {
      WarnMsg1("Unusual gamma, %f, passed to ImageApplyGamma\n", gamma);
   }

   float oneOverGamma = 1.0f / gamma;

   byte gammaLookUp[256];
   for (int i = 0; i < 256; ++i)
   {
      float value = (255 * pow(static_cast<float>(i) / 255, oneOverGamma)) + 0.5f;
      value = Min(value, 255);
      gammaLookUp[i] = (byte)FloatToInt(value);
   }

   for (uint j = y; j < (y + h); ++j)
   {
      for (uint i = x; i < (x + w); ++i)
      {
         byte rgba[4];
         pImage->GetPixel(i, j, rgba);

         rgba[0] = gammaLookUp[rgba[0]];
         rgba[1] = gammaLookUp[rgba[1]];
         rgba[2] = gammaLookUp[rgba[2]];
         rgba[3] = gammaLookUp[rgba[3]];

         pImage->SetPixel(i, j, rgba);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
