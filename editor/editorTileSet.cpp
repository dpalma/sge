///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileSet.h"
#include "BitmapUtils.h"

#include "resourceapi.h"
#include "imagedata.h"
#include "globalobj.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTileSet
//

///////////////////////////////////////

cEditorTileSet::cEditorTileSet(const tChar * pszName, const std::vector<cStr> & textures)
 : m_name(pszName != NULL ? pszName : ""),
   m_textures(textures.size())
{
   std::copy(textures.begin(), textures.end(), m_textures.begin());
}

///////////////////////////////////////

cEditorTileSet::~cEditorTileSet()
{
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

tResult cEditorTileSet::GetTileCount(uint * pTileCount) const
{
   if (pTileCount != NULL)
   {
      *pTileCount = m_textures.size();
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cEditorTileSet::GetTileTexture(uint iTile, cStr * pTexture) const
{
   if (iTile >= m_textures.size())
   {
      return E_INVALIDARG;
   }
   if (pTexture == NULL)
   {
      return E_POINTER;
   }
   *pTexture = m_textures[iTile];
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSet::GetTileName(uint iTile, cStr * pName) const
{
   if (iTile >= m_textures.size())
   {
      return E_INVALIDARG;
   }
   if (pName == NULL)
   {
      return E_POINTER;
   }
   *pName = m_textures[iTile];
   int iDot = pName->rfind('.');
   if (iDot != cStr::npos)
   {
      pName->erase(iDot);
   }
   (*pName)[0] = toupper(pName->at(0));
   return S_OK;
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
      // Return a copy
      *phImageList = ImageList_Duplicate(f->second);
      return S_OK;
   }

   HIMAGELIST hImageList = ImageList_Create(dimension, dimension, ILC_COLOR24, m_textures.size(), 0);
   if (hImageList == NULL)
   {
      return E_FAIL;
   }

   UseGlobal(ResourceManager);

   std::vector<cStr>::iterator iter = m_textures.begin();
   std::vector<cStr>::iterator end = m_textures.end();
   for (; iter != end; ++iter)
   {
      HBITMAP hbm = NULL;
      if (pResourceManager->Load(iter->c_str(), kRT_HBitmap, NULL, (void**)&hbm) == S_OK)
      {
         BITMAP bm;
         Verify(GetObject(hbm, sizeof(bm), &bm));
         HBITMAP hSizedBm = StretchCopyBitmap(dimension, dimension, hbm, 0, 0,
            min(dimension, (uint)bm.bmWidth), min(dimension, (uint)bm.bmHeight));
         if (hSizedBm != NULL)
         {
            Verify(ImageList_Add(hImageList, hSizedBm, NULL) >= 0);
            DeleteObject(hSizedBm);
         }
      }
   }

   m_imageLists.insert(std::make_pair(dimension, hImageList));

   // Return a copy
   *phImageList = ImageList_Duplicate(hImageList);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult EditorTileSetCreate(const char * pszName,
                            const std::vector<cStr> & textures,
                            IEditorTileSet * * ppTileSet)
{
   if (pszName == NULL || ppTileSet == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cEditorTileSet> pTileSet(new cEditorTileSet(pszName, textures));
   if (!pTileSet)
   {
      return E_OUTOFMEMORY;
   }

   *ppTileSet = CTAddRef(static_cast<IEditorTileSet *>(pTileSet));
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
