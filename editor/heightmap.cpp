///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resmgr.h"
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
   UseGlobal(ResourceManager);

   cImageData * pNewImage = ImageLoad(pResourceManager, pszFilename);
   if (pNewImage == NULL)
      return false;

   // images used as height data must be square and grayscale format
   if (pNewImage->GetWidth() != pNewImage->GetHeight() ||
       pNewImage->GetPixelFormat() != kPF_Grayscale)
   {
      delete pNewImage;
      return false;
   }

   Destroy();
   m_pImage = pNewImage;
   m_size = m_pImage->GetWidth();
   m_pData = (uint8 *)m_pImage->GetData();
   return true;
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
