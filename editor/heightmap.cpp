///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourceapi.h"
#include "globalobj.h"
#include "imagedata.h"
#include "heightmap.h"

#include "dbgalloc.h" // must be last header

F_DECLARE_INTERFACE(IResourceManager);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHeightMap
//

///////////////////////////////////////

cHeightMap::cHeightMap(float scale)
 : m_pImage(NULL),
   m_scale(scale),
   m_size(0),
   m_pData(NULL)
{
}

///////////////////////////////////////

cHeightMap::~cHeightMap()
{
   Destroy();
}

///////////////////////////////////////

bool cHeightMap::Load(const char * pszFilename)
{
   cAutoIPtr<IResource> pRes;
   UseGlobal(ResourceManager2);
   if (pResourceManager2->Load(tResKey(pszFilename, kRC_Image), &pRes) == S_OK)
   {
      cImageData * pHeightImage = NULL;
      if (pRes->GetData((void**)&pHeightImage) == S_OK && pHeightImage != NULL)
      {
         // images used as height data must be square and grayscale format
         if (pHeightImage->GetWidth() == pHeightImage->GetHeight() &&
             pHeightImage->GetPixelFormat() == kPF_Grayscale)
         {
            Destroy();
            m_pImage = pHeightImage;
            m_size = m_pImage->GetWidth();
            m_pData = (uint8 *)m_pImage->GetData();
            return true;
         }

         delete pHeightImage;
      }
   }

   return false;
}

///////////////////////////////////////

void cHeightMap::Destroy()
{
   delete m_pImage;
   m_pImage = NULL;
   m_size = 0;
   m_pData = NULL;
}

///////////////////////////////////////////////////////////////////////////////
