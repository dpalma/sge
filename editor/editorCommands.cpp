/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCommands.h"
#include "terrain.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

////////////////////////////////////////

cTerrainTileCommand::cTerrainTileCommand(cTerrain * pTerrain, uint ix, uint iz, uint tile)
 : m_pTerrain(pTerrain),
   m_ix(ix),
   m_iz(iz),
   m_tile(tile),
   m_oldTile(tile)
{
}

////////////////////////////////////////

cTerrainTileCommand::~cTerrainTileCommand()
{
}

////////////////////////////////////////

tResult cTerrainTileCommand::Do()
{
   if (m_pTerrain != NULL)
   {
      cTerrainTile * pTile = m_pTerrain->GetTile(m_ix, m_iz);
      if (pTile != NULL)
      {
         m_oldTile = pTile->GetTile();
         pTile->SetTile(m_tile);
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainTileCommand::CanUndo()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileCommand::Undo()
{
   if (m_pTerrain != NULL)
   {
      cTerrainTile * pTile = m_pTerrain->GetTile(m_ix, m_iz);
      if (pTile != NULL)
      {
         pTile->SetTile(m_oldTile);
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainTileCommand::GetLabel(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   *pLabel = "Terrain Texture";

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
