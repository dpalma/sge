///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "gcommon.h"
#include "image.h"
#include "pixelformat.h"
#include "heightmap.h"

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
   cImage * pNewImage = ImageLoad(AccessResourceManager(), pszFilename);
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
