///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileSet.h"
#include "editorTile.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTileSet
//

///////////////////////////////////////

cEditorTileSet::cEditorTileSet(const tChar * pszName)
 : m_name(pszName != NULL ? pszName : "")
{
}

///////////////////////////////////////

cEditorTileSet::~cEditorTileSet()
{
   std::for_each(m_tiles.begin(), m_tiles.end(), CTInterfaceMethod(&IEditorTile::Release));
}

///////////////////////////////////////

tResult cEditorTileSet::GetName(cStr * pName) const
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

tResult cEditorTileSet::AddTile(const tChar * pszName,
                                const tChar * pszTexture,
                                int horzImages,
                                int vertImages)
{
//   DebugMsg4("AddTile(\"%s\", \"%s\", %d, %d)\n", pszName, pszTexture, horzImages, vertImages);
   cAutoIPtr<IEditorTile> pTile(new cEditorTile(pszName, pszTexture, horzImages, vertImages));
   if (!pTile)
   {
      return E_OUTOFMEMORY;
   }
   else
   {
      tTiles::iterator iter;
      for (iter = m_tiles.begin(); iter != m_tiles.end(); iter++)
      {
         cStr name, texture;
         Verify((*iter)->GetName(&name) == S_OK);
         Verify((*iter)->GetTexture(&texture) == S_OK);
         if (strcmp(pszName, name.c_str()) == 0
            || strcmp(pszTexture, texture.c_str()) == 0)
         {
            return S_FALSE;
         }
      }
      m_tiles.push_back(CTAddRef(pTile));
      return S_OK;
   }
}

///////////////////////////////////////

tResult cEditorTileSet::GetTileCount(uint * pTileCount) const
{
   if (pTileCount != NULL)
   {
      *pTileCount = m_tiles.size();
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cEditorTileSet::GetTile(uint index, IEditorTile * * ppTile)
{
   if (index >= m_tiles.size())
   {
      return E_INVALIDARG;
   }
   if (ppTile == NULL)
   {
      return E_POINTER;
   }
   *ppTile = CTAddRef(m_tiles[index]);
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSet::GetMaterial(IMaterial * * ppMaterial)
{
   if (!m_pMaterial)
   {
      m_pMaterial = MaterialCreate();
      if (!m_pMaterial)
      {
         return E_OUTOFMEMORY;
      }

      int index = 0;
      tTiles::iterator iter;
      for (iter = m_tiles.begin(); (iter != m_tiles.end()) && (index < kMaxTextures); iter++, index++)
      {
         cAutoIPtr<ITexture> pTexture;
         if ((*iter)->GetTexture(&pTexture) == S_OK)
         {
            m_pMaterial->SetTexture(index, pTexture);
         }
      }
   }

   return m_pMaterial.GetPointer(ppMaterial);
}

///////////////////////////////////////

tResult cEditorTileSet::GetImageList(uint dimension, HIMAGELIST * phImageList)
{
   return E_NOTIMPL; // TODO
}

///////////////////////////////////////////////////////////////////////////////
