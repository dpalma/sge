///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTile.h"
#include "BitmapUtils.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTile
//

///////////////////////////////////////

cEditorTile::cEditorTile(const tChar * pszName, const tChar * pszTexture,
                         int horzImages, int vertImages)
 : m_name(pszName != NULL ? pszName : ""),
   m_texture(pszTexture != NULL ? pszTexture : ""),
   m_horzImages(horzImages),
   m_vertImages(vertImages),
   m_pImageData(NULL),
   m_hBitmap(NULL),
   m_bLoadBitmapFailed(false)
{
}

///////////////////////////////////////

cEditorTile::~cEditorTile()
{
   tBitmaps::iterator iter;
   for (iter = m_bitmaps.begin(); iter != m_bitmaps.end(); iter++)
   {
      if (iter->second != NULL)
      {
         DeleteObject(iter->second);
      }
   }
   m_bitmaps.clear();

   delete m_pImageData;
   m_pImageData = NULL;

   if (m_hBitmap != NULL)
   {
      DeleteObject(m_hBitmap);
      m_hBitmap = NULL;
   }
}

///////////////////////////////////////

tResult cEditorTile::GetName(cStr * pName) const
{
   if (pName != NULL)
   {
      *pName = m_name;
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cEditorTile::GetTexture(cStr * pTexture) const
{
   if (pTexture != NULL)
   {
      *pTexture = m_texture;
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

uint cEditorTile::GetHorizontalImageCount() const
{
   return m_horzImages;
}

///////////////////////////////////////

uint cEditorTile::GetVerticalImageCount() const
{
   return m_vertImages;
}

///////////////////////////////////////

tResult cEditorTile::GetTexture(ITexture * * ppTexture)
{
   if (!m_pTexture && !m_texture.empty())
   {
      UseGlobal(TextureManager);
      if (FAILED(pTextureManager->GetTexture(m_texture.c_str(), &m_pTexture)))
      {
         return E_FAIL;
      }
   }

   return m_pTexture.GetPointer(ppTexture);
}

///////////////////////////////////////

tResult cEditorTile::GetBitmap(uint dimension, bool bEntire, HBITMAP * phBitmap)
{
   if (dimension == 0)
   {
      return E_INVALIDARG;
   }

   if (phBitmap == NULL)
   {
      return E_POINTER;
   }

   LazyInit();

   if (m_hBitmap == NULL)
   {
      return E_FAIL;
   }

   Assert(m_pImageData != NULL);

   tBitmaps::iterator f = m_bitmaps.find(dimension);
   if (f != m_bitmaps.end())
   {
      *phBitmap = f->second;
      return S_OK;
   }

   uint tileWidth = m_pImageData->GetWidth() / m_horzImages;
   uint tileHeight = m_pImageData->GetHeight() / m_vertImages;

   HBITMAP hbm = StretchCopyBitmap(dimension, dimension, m_hBitmap, 0, 0,
      bEntire ? tileWidth : min(dimension, tileWidth),
      bEntire ? tileHeight : min(dimension, tileHeight));

   if (hbm == NULL)
   {
      return E_FAIL;
   }

   m_bitmaps.insert(std::make_pair(dimension, hbm));

   *phBitmap = hbm;

   return S_OK;
}

///////////////////////////////////////

void cEditorTile::LazyInit()
{
   if (!m_texture.empty())
   {
      if (m_pImageData == NULL && !m_bLoadBitmapFailed)
      {
         Assert(m_hBitmap == NULL);

         UseGlobal(ResourceManager2);
         if (pResourceManager2->Load(tResKey(m_texture.c_str(), kRC_Image), (void**)&m_pImageData) == S_OK)
         {
            m_bLoadBitmapFailed = !LoadBitmap(m_pImageData, &m_hBitmap);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
