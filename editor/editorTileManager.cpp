///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileManager.h"

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

   return E_NOTIMPL;
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

void EditorTileManagerCreate()
{
   cAutoIPtr<IEditorTileManager>(new cEditorTileManager);
}

///////////////////////////////////////////////////////////////////////////////
