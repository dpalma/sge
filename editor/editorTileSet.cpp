///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileSet.h"
#include "editorTile.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

   tImageLists::iterator iter;
   for (iter = m_imageLists.begin(); iter != m_imageLists.end(); iter++)
   {
      Verify(ImageList_Destroy(iter->second));
   }
   m_imageLists.clear();
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

tResult cEditorTileSet::GetTileTexture(uint iTile, ITexture * * ppTexture)
{
   if (iTile >= m_tiles.size())
   {
      return E_INVALIDARG;
   }
   if (ppTexture == NULL)
   {
      return E_POINTER;
   }
   return m_tiles[iTile]->GetTexture(ppTexture);
}

///////////////////////////////////////

tResult cEditorTileSet::GetImageList(uint dimension, HIMAGELIST * phImageList)
{
   if (dimension == 0)
   {
      return E_INVALIDARG;
   }

   if (phImageList == NULL)
   {
      return E_POINTER;
   }

   tImageLists::iterator f = m_imageLists.find(dimension);
   if (f != m_imageLists.end())
   {
      *phImageList = f->second;
      return S_OK;
   }

   HIMAGELIST hImageList = ImageList_Create(dimension, dimension, ILC_COLOR24, m_tiles.size(), 0);
   if (hImageList == NULL)
   {
      return E_FAIL;
   }

   tTiles::iterator iter;
   for (iter = m_tiles.begin(); iter != m_tiles.end(); iter++)
   {
      HBITMAP hTileBitmap;
      if ((*iter)->GetBitmap(dimension, false, &hTileBitmap) == S_OK)
      {
         Verify(ImageList_Add(hImageList, hTileBitmap, NULL) >= 0);
      }
   }

   m_imageLists.insert(std::make_pair(dimension, hImageList));

   *phImageList = hImageList;

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
