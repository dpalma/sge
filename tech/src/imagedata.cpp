///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imagedata.h"
#include "resmgr.h"
#include "readwriteapi.h"
#include "filespec.h"

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
// CLASS: cImageData
//

///////////////////////////////////////

cImageData::cImageData()
 : m_width(0), m_height(0), m_pData(NULL), m_pixelFormat(kPF_ERROR)
{
}

///////////////////////////////////////

cImageData::~cImageData()
{
   Destroy();
}
   
///////////////////////////////////////

bool cImageData::Create(int width, int height, ePixelFormat pixelFormat, void * pData /*= NULL*/)
{
   if (m_pData != NULL)
   {
      DebugMsg("WARNING: Create called on non-empty cImage object\n");
      return false;
   }

   m_pixelFormat = pixelFormat;

   m_width = width;
   m_height = height;

   uint memSize = BytesPerPixel(GetPixelFormat()) * width * height;
   if (memSize == 0)
   {
      DebugMsg3("WARNING: Product of %d, %d, %d is zero\n",
         BytesPerPixel(GetPixelFormat()), width, height);
      return false;
   }

   m_pData = new byte[memSize];

   if (pData != NULL)
   {
      memcpy(GetData(), pData, memSize);
   }

   return true;
}

///////////////////////////////////////

void cImageData::Destroy()
{
   delete [] m_pData;
   m_pData = NULL;
}

///////////////////////////////////////////////////////////////////////////////

cImageData * LoadBmp(IReader *); // bmp.cpp
cImageData * LoadTarga(IReader *); // tga.cpp

cImageData * ImageLoad(IResourceManager * pResMgr, const char * pszFile)
{
   typedef cImageData * (* tImageLoadFn)(IReader *);

   static const struct
   {
      const char * ext;
      tImageLoadFn pfn;
   }
   imageFileLoaders[] =
   {
      { "bmp", LoadBmp },
      { "tga", LoadTarga },
   };

   Assert(pResMgr != NULL);
   Assert(pszFile != NULL);

   cFileSpec file(pszFile);

   for (int i = 0; i < _countof(imageFileLoaders); i++)
   {
      if (stricmp(imageFileLoaders[i].ext, file.GetFileExt()) == 0)
      {
         cAutoIPtr<IReader> pReader = pResMgr->Find(pszFile);
         if (!pReader)
         {
            return NULL;
         }

         cImageData * pImage = (*imageFileLoaders[i].pfn)(pReader);
         if (pImage != NULL)
         {
            return pImage;
         }
      }
   }

   DebugMsg1("Unsupported image file format: \"%s\"\n", file.GetFileExt());
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
