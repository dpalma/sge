///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileSets.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LOG_DEFINE_CHANNEL(EditorTileSets);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorTileSets
//

///////////////////////////////////////

cEditorTileSets::cEditorTileSets()
{
}

///////////////////////////////////////

cEditorTileSets::~cEditorTileSets()
{
}

///////////////////////////////////////

tResult cEditorTileSets::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::Term()
{
   m_tileSets.clear();
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::AddTileSet(const tChar * pszTileSet)
{
   if (pszTileSet == NULL)
   {
      return E_POINTER;
   }

   if (HasTileSet(pszTileSet))
   {
      return S_FALSE;
   }

   m_tileSets.push_back(pszTileSet);
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::GetTileSetCount(uint * pTileSetCount) const
{
   if (pTileSetCount == NULL)
   {
      return E_POINTER;
   }
   *pTileSetCount = m_tileSets.size();
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::GetTileSet(uint index, cStr * pTileSet) const
{
   if (index >= m_tileSets.size())
   {
      return E_INVALIDARG;
   }

   if (pTileSet == NULL)
   {
      return E_POINTER;
   }

   *pTileSet = m_tileSets[index];
   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::SetDefaultTileSet(const tChar * pszTileSet)
{
   if (pszTileSet != NULL)
   {
      if (m_defaultTileSet.compare(pszTileSet) == 0)
      {
         return S_FALSE;
      }
      else
      {
         m_defaultTileSet = pszTileSet;
         ForEachConnection(&IEditorTileSetsListener::OnSetDefaultTileSet, pszTileSet);
      }
   }
   else
   {
      m_defaultTileSet.erase();
      ForEachConnection(&IEditorTileSetsListener::OnSetDefaultTileSet, static_cast<const tChar *>(NULL));
   }

   return S_OK;
}

///////////////////////////////////////

tResult cEditorTileSets::GetDefaultTileSet(cStr * pTileSet) const
{
   if (pTileSet == NULL)
   {
      return E_POINTER;
   }
   *pTileSet = m_defaultTileSet;
   return S_OK;
}

///////////////////////////////////////

bool cEditorTileSets::HasTileSet(const tChar * pszTileSet) const
{
   if (pszTileSet != NULL)
   {
      tStrings::const_iterator iter = m_tileSets.begin();
      tStrings::const_iterator end = m_tileSets.end();
      for (; iter != end; iter++)
      {
         if (iter->compare(pszTileSet) == 0)
         {
            return true;
         }
      }
   }
   return false;
}

///////////////////////////////////////

void EditorTileSetsCreate()
{
   cAutoIPtr<IEditorTileSets> p(new cEditorTileSets);
   RegisterGlobalObject(IID_IEditorTileSets, static_cast<IEditorTileSets*>(p));
}

///////////////////////////////////////////////////////////////////////////////
