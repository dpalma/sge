///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileManager.h"
#include "editorTileSet.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(EditorTileManager);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorTileManager
//

///////////////////////////////////////

cEditorTileManager::cEditorTileManager()
{
}

///////////////////////////////////////

tResult cEditorTileManager::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileManager::Term()
{
   tTileSetMap::iterator iter;
   for (iter = m_tileSetMap.begin(); iter != m_tileSetMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_tileSetMap.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileManager::CreateTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet)
{
   if (pszName == NULL || ppTileSet == NULL)
   {
      return E_POINTER;
   }

   tTileSetMap::iterator f = m_tileSetMap.find(pszName);

   if (f != m_tileSetMap.end())
   {
      *ppTileSet = CTAddRef(f->second);
      return S_FALSE;
   }

   cAutoIPtr<IEditorTileSet> pTileSet(new cEditorTileSet(pszName));

   if (!pTileSet)
   {
      return E_OUTOFMEMORY;
   }

   m_tileSetMap[cStr(pszName)] = CTAddRef(static_cast<IEditorTileSet *>(pTileSet));

   *ppTileSet = CTAddRef(static_cast<IEditorTileSet *>(pTileSet));
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileManager::GetTileSet(const tChar * pszName, IEditorTileSet * * ppTileSet)
{
   if (pszName == NULL || ppTileSet == NULL)
   {
      return E_POINTER;
   }

   tTileSetMap::iterator f = m_tileSetMap.find(pszName);

   if (f != m_tileSetMap.end())
   {
      *ppTileSet = CTAddRef(f->second);
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cEditorTileManager::GetDefaultTileSet(IEditorTileSet * * ppTileSet)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cEditorTileManager::SetDefaultTileSet(const tChar * pszName)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

void EditorTileManagerCreate()
{
   cAutoIPtr<IEditorTileManager>(new cEditorTileManager);
}

///////////////////////////////////////////////////////////////////////////////
