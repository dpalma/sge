////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine/terrainapi.h"

#include "tech/imageapi.h"
#include "tech/globalobj.h"
#include "tech/resourceapi.h"
#include "tech/techmath.h"

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

tResult HeightMapCreateFixed(float heightValue, IHeightMap * * ppHeightMap)
{
   if (ppHeightMap == NULL)
   {
      return E_POINTER;
   }

   class cFixedHeightMap : public cComObject<IMPLEMENTS(IHeightMap)>
   {
   public:
      cFixedHeightMap(float heightValue) : m_heightValue(heightValue)
      {
      }

      virtual float GetNormalizedHeight(float /*nx*/, float /*nz*/) const
      {
         return m_heightValue;
      }

   private:
      float m_heightValue;
   };

   *ppHeightMap = static_cast<IHeightMap *>(new cFixedHeightMap(heightValue));
   if (*ppHeightMap == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

tResult HeightMapLoad(const tChar * pszHeightData, IHeightMap * * ppHeightMap)
{
   if (pszHeightData == NULL || ppHeightMap == NULL)
   {
      return E_POINTER;
   }

   class cHeightMap : public cComObject<IMPLEMENTS(IHeightMap)>
   {
   public:
      cHeightMap(IImage * pHeightMap)
       : m_pHeightMap(pHeightMap)
      {
         Assert(pHeightMap != NULL);
      }

      ~cHeightMap()
      {
         m_pHeightMap = NULL; // Don't delete this--it's a cached resource
      }

      virtual float GetNormalizedHeight(float nx, float nz) const
      {
         Assert(m_pHeightMap != NULL);

         // support only grayscale images for now
         if (m_pHeightMap->GetPixelFormat() != kPF_Grayscale)
         {
            return 0;
         }

         if ((nx < 0) || (nx > 1) || (nz < 0) || (nz > 1))
         {
            return 0;
         }

         uint x = FloatToInt(nx * m_pHeightMap->GetWidth());
         uint z = FloatToInt(nz * m_pHeightMap->GetHeight());

         const uint8 * pData = reinterpret_cast<const uint8 *>(m_pHeightMap->GetData());

         uint8 sample = pData[(z * m_pHeightMap->GetWidth()) + x];

         return static_cast<float>(sample) / 255.0f;
      }

   private:
      IImage * m_pHeightMap;
   };

   IImage * pHeightData = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszHeightData, kRT_Image, NULL, (void**)&pHeightData) != S_OK)
   {
      return E_FAIL;
   }

   *ppHeightMap = static_cast<IHeightMap *>(new cHeightMap(pHeightData));
   if (*ppHeightMap == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
