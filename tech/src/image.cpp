///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imagedata.h" // TEMPORARY
#include "image.h"

#include "color.h"
#include "filespec.h"
#include "globalobj.h"
#include "resourceapi.h"
#include "readwriteapi.h"

#include <cstring>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static uint BytesPerPixel(ePixelFormat pf)
{
   switch (pf)
   {
      case kPF_Grayscale:
      case kPF_ColorMapped:
         return 1;

      case kPF_RGB555:
      case kPF_BGR555:
      case kPF_RGB565:
      case kPF_BGR565:
      case kPF_RGBA1555:
      case kPF_BGRA1555:
         return 2;

      case kPF_RGB888:
      case kPF_BGR888:
         return 3;

      case kPF_RGBA8888:
      case kPF_BGRA8888:
         return 4;

      default:
         return 0;
   }
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
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cImage::SetPixel(uint x, uint y, const cColor & pixel)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cImage::Clone(IImage * * ppImage)
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////

void * ImageLoad(void * pData, int dataLength, void * param)
{
   cImageData * pImageData = reinterpret_cast<cImageData*>(pData);
   if (pImageData != NULL)
   {
      IImage * pImage = NULL;
      if (ImageCreate(pImageData->GetWidth(), pImageData->GetWidth(), pImageData->GetPixelFormat(),
         pImageData->GetData(), &pImage) == S_OK)
      {
         return pImage;
      }
   }
   return NULL;
}

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
      if (pResourceManager->RegisterFormat(kRT_Image, MAKERESOURCETYPE(kRC_Image),
         NULL, NULL, ImageLoad, ImageUnload) == S_OK)
      {
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

   cAutoIPtr<cImage> pImage(new cImage(width, height, pixelFormat, pImageData));
   if (!pImage)
   {
      return E_OUTOFMEMORY;
   }

   *ppImage = CTAddRef(static_cast<IImage*>(pImage));
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
