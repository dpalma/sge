/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTypes.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "globalobj.h"
#include "techmath.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettings
//

////////////////////////////////////////

cMapSettings::cMapSettings()
 : m_xDimension(0),
   m_zDimension(0),
   m_tileSet(""),
   m_heightData(kHeightData_None),
   m_heightMapFile(""),
   m_pHeightImageData(NULL)
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(uint xDimension,
                           uint zDimension,
                           const tChar * pszTileSet,
                           eHeightData heightData,
                           const tChar * pszHeightMapFile)
 : m_xDimension(xDimension),
   m_zDimension(zDimension),
   m_tileSet(pszTileSet != NULL ? pszTileSet : ""),
   m_heightData(heightData),
   m_heightMapFile(pszHeightMapFile != NULL ? pszHeightMapFile : ""),
   m_pHeightImageData(NULL)
{
}

////////////////////////////////////////

cMapSettings::cMapSettings(const cMapSettings & mapSettings)
 : m_xDimension(mapSettings.m_xDimension),
   m_zDimension(mapSettings.m_zDimension),
   m_tileSet(mapSettings.m_tileSet),
   m_heightData(mapSettings.m_heightData),
   m_heightMapFile(mapSettings.m_heightMapFile),
   m_pHeightImageData(NULL)
{
}

////////////////////////////////////////

cMapSettings::~cMapSettings()
{
   delete m_pHeightImageData, m_pHeightImageData = NULL;
}

////////////////////////////////////////

const cMapSettings & cMapSettings::operator =(const cMapSettings & mapSettings)
{
   m_xDimension = mapSettings.m_xDimension;
   m_zDimension = mapSettings.m_zDimension;
   m_tileSet = mapSettings.m_tileSet;
   m_heightData = mapSettings.m_heightData;
   m_heightMapFile = mapSettings.m_heightMapFile;
   return *this;
}

////////////////////////////////////////

float cMapSettings::GetNormalizedHeight(float nx, float nz) const
{
   if (GetHeightData() != kHeightData_HeightMap)
   {
      return 0;
   }

   if (m_pHeightImageData == NULL)
   {
      cAutoIPtr<IResource> pRes;
      UseGlobal(ResourceManager2);
      if (pResourceManager2->Load(tResKey(GetHeightMap(), kRC_Image), &pRes) == S_OK)
      {
         if ((pRes->GetData((void**)&m_pHeightImageData) != S_OK)
            || (m_pHeightImageData == NULL))
         {
            return 0;
         }
      }
   }

   Assert(m_pHeightImageData != NULL);

   // support only grayscale images for now
   if (m_pHeightImageData->GetPixelFormat() != kPF_Grayscale)
   {
      return 0;
   }

   if ((nx < 0) || (nx > 1) || (nz < 0) || (nz > 1))
   {
      return 0;
   }

   uint x = Round(nx * m_pHeightImageData->GetWidth());
   uint z = Round(nz * m_pHeightImageData->GetHeight());

   uint8 * pData = reinterpret_cast<uint8 *>(m_pHeightImageData);

   uint8 sample = pData[(z * m_pHeightImageData->GetWidth()) + x];

   return static_cast<float>(sample) / 255.0f;
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorKeyEvent
//

cEditorKeyEvent::cEditorKeyEvent(WPARAM wParam, LPARAM lParam)
 : m_char(wParam), m_repeats(LOWORD(lParam)), m_flags(HIWORD(lParam))
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseEvent
//

cEditorMouseEvent::cEditorMouseEvent(WPARAM wParam, LPARAM lParam)
 : m_flags(wParam), m_point(lParam)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseWheelEvent
//

cEditorMouseWheelEvent::cEditorMouseWheelEvent(WPARAM wParam, LPARAM lParam)
 : cEditorMouseEvent(LOWORD(wParam), lParam), m_zDelta(HIWORD(wParam))
{
}


/////////////////////////////////////////////////////////////////////////////
