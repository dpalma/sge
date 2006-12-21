///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "image.h"

#include "tech/globalobj.h"
#include "tech/resourceapi.h"
#include "tech/techmath.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "tech/dbgalloc.h" // must be last header

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


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRGBA
//

////////////////////////////////////////

cRGBA::cRGBA()
{
}

////////////////////////////////////////

cRGBA::cRGBA(byte r, byte g, byte b)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = 1;
}

////////////////////////////////////////

cRGBA::cRGBA(byte r, byte g, byte b, byte a)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = a;
}

////////////////////////////////////////

cRGBA::cRGBA(const byte rgba[4])
{
   memcpy(&m_rgba[0], &rgba[0], sizeof(m_rgba));
}

////////////////////////////////////////
// assumes all float values are between 0 and 1

cRGBA::cRGBA(const float rgba[4])
{
   m_rgba[0] = static_cast<byte>(rgba[0] * 255.0f);
   m_rgba[1] = static_cast<byte>(rgba[1] * 255.0f);
   m_rgba[2] = static_cast<byte>(rgba[2] * 255.0f);
   m_rgba[3] = static_cast<byte>(rgba[3] * 255.0f);
}

////////////////////////////////////////

cRGBA::cRGBA(const cRGBA & other)
{
   memcpy(&m_rgba[0], &other.m_rgba[0], sizeof(m_rgba));
}

////////////////////////////////////////

const cRGBA & cRGBA::operator =(const cRGBA & other)
{
   memcpy(&m_rgba[0], &other.m_rgba[0], sizeof(m_rgba));
   return *this;
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageBase
//

/////////////////////////////////////////

cImageBase::cImageBase()
 : m_width(0)
 , m_height(0)
 , m_pixelFormat(kPF_ERROR)
 , m_pData(NULL)
{
}

/////////////////////////////////////////

cImageBase::~cImageBase()
{
   delete [] m_pData;
   m_pData = NULL;
}

/////////////////////////////////////////

uint cImageBase::GetWidth() const
{
   return m_width;
}

/////////////////////////////////////////

uint cImageBase::GetHeight() const
{
   return m_height;
}

/////////////////////////////////////////

ePixelFormat cImageBase::GetPixelFormat() const
{
   return m_pixelFormat;
}

/////////////////////////////////////////

const void * cImageBase::GetData() const
{
   return m_pData;
}

////////////////////////////////////////

tResult cImageBase::Clone(IImage * * ppImage)
{
   return ImageCreate(GetWidth(), GetHeight(), GetPixelFormat(), GetData(), ppImage);
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPixelRGB565
//

class cPixelRGB565
{
public:
   static uint BytesPerPixel();
   static void GetPixel(const byte * pPixel, byte rgba[4]);
   static void SetPixel(byte * pPixel, const byte rgba[4]);
};

////////////////////////////////////////

uint cPixelRGB565::BytesPerPixel()
{
   return 2;
}

////////////////////////////////////////

void cPixelRGB565::GetPixel(const byte * pPixel, byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   uint16 pixel = *(uint16 *)pPixel;
   rgba[0] = ((pixel >> 11) & 31) << 3;
   rgba[1] = ((pixel >> 5) & 63) << 2;
   rgba[2] = (pixel & 31) << 3;
   rgba[3] = 1;
}

////////////////////////////////////////

void cPixelRGB565::SetPixel(byte * pPixel, const byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   uint16 * pPixel16 = (uint16 *)pPixel;
   *pPixel16 = (((uint16)rgba[0] >> 3) << 11) | (((uint16)rgba[1] >> 2) << 5)  | ((uint16)rgba[2] >> 3);
   // RGB565 is a 3-component pixel format so ignore alpha
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPixelRGB888
//

class cPixelRGB888
{
public:
   static uint BytesPerPixel();
   static void GetPixel(const byte * pPixel, byte rgba[4]);
   static void SetPixel(byte * pPixel, const byte rgba[4]);
};

////////////////////////////////////////

uint cPixelRGB888::BytesPerPixel()
{
   return 3;
}

////////////////////////////////////////

void cPixelRGB888::GetPixel(const byte * pPixel, byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   rgba[0] = pPixel[0];
   rgba[1] = pPixel[1];
   rgba[2] = pPixel[2];
   rgba[3] = 1;
}

////////////////////////////////////////

void cPixelRGB888::SetPixel(byte * pPixel, const byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   pPixel[0] = rgba[0];
   pPixel[1] = rgba[1];
   pPixel[2] = rgba[2];
   // RGB888 is a 3-component pixel format so ignore alpha
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPixelBGR888
//

class cPixelBGR888
{
public:
   static uint BytesPerPixel();
   static void GetPixel(const byte * pPixel, byte rgba[4]);
   static void SetPixel(byte * pPixel, const byte rgba[4]);
};

////////////////////////////////////////

uint cPixelBGR888::BytesPerPixel()
{
   return 3;
}

////////////////////////////////////////

void cPixelBGR888::GetPixel(const byte * pPixel, byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   rgba[0] = pPixel[2];
   rgba[1] = pPixel[1];
   rgba[2] = pPixel[0];
   rgba[3] = 1;
}

////////////////////////////////////////

void cPixelBGR888::SetPixel(byte * pPixel, const byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   pPixel[0] = rgba[2];
   pPixel[1] = rgba[1];
   pPixel[2] = rgba[0];
   // RGB888 is a 3-component pixel format so ignore alpha
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPixelRGBA8888
//

class cPixelRGBA8888
{
public:
   static uint BytesPerPixel();
   static void GetPixel(const byte * pPixel, byte rgba[4]);
   static void SetPixel(byte * pPixel, const byte rgba[4]);
};

////////////////////////////////////////

uint cPixelRGBA8888::BytesPerPixel()
{
   return 4;
}

////////////////////////////////////////

void cPixelRGBA8888::GetPixel(const byte * pPixel, byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   rgba[0] = pPixel[0];
   rgba[1] = pPixel[1];
   rgba[2] = pPixel[2];
   rgba[3] = pPixel[3];
}

////////////////////////////////////////

void cPixelRGBA8888::SetPixel(byte * pPixel, const byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   pPixel[0] = rgba[0];
   pPixel[1] = rgba[1];
   pPixel[2] = rgba[2];
   pPixel[3] = rgba[3];
}


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPixelBGRA8888
//

class cPixelBGRA8888
{
public:
   static uint BytesPerPixel();
   static void GetPixel(const byte * pPixel, byte rgba[4]);
   static void SetPixel(byte * pPixel, const byte rgba[4]);
};

////////////////////////////////////////

uint cPixelBGRA8888::BytesPerPixel()
{
   return 4;
}

////////////////////////////////////////

void cPixelBGRA8888::GetPixel(const byte * pPixel, byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   rgba[0] = pPixel[2];
   rgba[1] = pPixel[1];
   rgba[2] = pPixel[0];
   rgba[3] = pPixel[3];
}

////////////////////////////////////////

void cPixelBGRA8888::SetPixel(byte * pPixel, const byte rgba[4])
{
   Assert(pPixel != NULL && rgba != NULL); // Error-checking should have happened by now
   pPixel[0] = rgba[2];
   pPixel[1] = rgba[1];
   pPixel[2] = rgba[0];
   pPixel[3] = rgba[3];
}


///////////////////////////////////////////////////////////////////////////////
// Windows DDB resource type

#ifdef _WIN32

////////////////////////////////////////

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

////////////////////////////////////////

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
extern void * JpgLoad(IReader * pReader);

////////////////////////////////////////

void ImageUnload(void * pData)
{
   reinterpret_cast<IImage*>(pData)->Release();
}

////////////////////////////////////////

tResult ImageRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_Image, _T("bmp"), BmpLoad, NULL, ImageUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Image, _T("jpeg"), JpgLoad, NULL, ImageUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Image, _T("jpg"), JpgLoad, NULL, ImageUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Image, _T("tga"), TargaLoad, NULL, ImageUnload) == S_OK)
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

template <typename PIXEL>
tResult ImageCreate2(uint width, uint height, ePixelFormat pixelFormat, byte * pImageData, IImage * * ppImage)
{
   cAutoIPtr<IImage> pImage(static_cast<IImage*>(new cImageT<PIXEL>(width, height, pixelFormat, pImageData)));
   if (!pImage)
   {
      return E_OUTOFMEMORY;
   }

   return pImage.GetPointer(ppImage);
}

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

   tResult result = E_FAIL;

   if (pixelFormat == kPF_RGB565)
   {
      result = ImageCreate2<cPixelRGB565>(width, height, pixelFormat, pImageData, ppImage);
   }
   else if (pixelFormat == kPF_RGB888)
   {
      result = ImageCreate2<cPixelRGB888>(width, height, pixelFormat, pImageData, ppImage);
   }
   else if (pixelFormat == kPF_BGR888)
   {
      result = ImageCreate2<cPixelBGR888>(width, height, pixelFormat, pImageData, ppImage);
   }
   else if (pixelFormat == kPF_RGBA8888)
   {
      result = ImageCreate2<cPixelRGBA8888>(width, height, pixelFormat, pImageData, ppImage);
   }
   else if (pixelFormat == kPF_BGRA8888)
   {
      result = ImageCreate2<cPixelBGRA8888>(width, height, pixelFormat, pImageData, ppImage);
   }

   if (result != S_OK)
   {
      delete [] pImageData;
   }

   return result;
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
