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
   return GetTileSet(m_defaultTileSet.c_str(), ppTileSet);
}

///////////////////////////////////////

tResult cEditorTileManager::SetDefaultTileSet(const tChar * pszName)
{
   if (pszName != NULL)
   {
      m_defaultTileSet = pszName;

      cAutoIPtr<IEditorTileSet> pTileSet;
      if (GetTileSet(pszName, &pTileSet) == S_OK)
      {
         ForEachConnection(&IEditorTileManagerListener::OnDefaultTileSetChange,
            static_cast<IEditorTileSet *>(pTileSet));
      }
   }
   else
   {
      m_defaultTileSet.erase();

      ForEachConnection(&IEditorTileManagerListener::OnDefaultTileSetChange,
         static_cast<IEditorTileSet *>(NULL));
   }

   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileManager::GetTileSetCount(uint * pTileSets)
{
   if (pTileSets == NULL)
   {
      return E_POINTER;
   }
   else
   {
      *pTileSets = m_tileSetMap.size();
      return S_OK;
   }
}

///////////////////////////////////////

tResult cEditorTileManager::GetTileSet(uint index, IEditorTileSet * * ppTileSet)
{
   if (ppTileSet == NULL)
   {
      return E_POINTER;
   }

   if (index >= m_tileSetMap.size())
   {
      DebugMsg1("Index %d out of range in cEditorTileManager::GetTileSet()\n", index);
      *ppTileSet = NULL;
      return S_FALSE;
   }

   tTileSetMap::iterator iter = m_tileSetMap.begin();
   for (uint i = 0; i < index; i++, iter++)
   {
      // do nothing
   }

   if (iter == m_tileSetMap.end())
   {
      *ppTileSet = NULL;
      return S_FALSE;
   }

   *ppTileSet = CTAddRef(iter->second);

   return (*ppTileSet) != NULL ? S_OK : S_FALSE;
}

///////////////////////////////////////

void EditorTileManagerCreate()
{
   cAutoIPtr<IEditorTileManager>(new cEditorTileManager);
}

///////////////////////////////////////////////////////////////////////////////
