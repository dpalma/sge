/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCommands.h"
#include "terrain.h"

#include "resource.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

////////////////////////////////////////

cTerrainTileCommand::cTerrainTileCommand(ITerrainModel * pTerrain, uint ix, uint iz, uint tile, ulong stamp)
 : m_pModel(CTAddRef(pTerrain)),
   m_ix(ix),
   m_iz(iz),
   m_tile(tile),
   m_oldTile(tile),
   m_stamp(stamp)
{
}

////////////////////////////////////////

cTerrainTileCommand::~cTerrainTileCommand()
{
}

////////////////////////////////////////

tResult cTerrainTileCommand::Do()
{
   if (!!m_pModel)
   {
      return m_pModel->SetTileTerrain(m_ix,m_iz,m_tile,&m_oldTile);
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
   if (!!m_pModel)
   {
      uint t;
      tResult result = m_pModel->SetTileTerrain(m_ix,m_iz,m_oldTile,&t);
      Assert(t = m_tile);
      return result;
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

   CString label;
   if (!label.LoadString(IDS_TERRAIN_TILE_COMMAND_LABEL))
   {
      return E_FAIL;
   }

   *pLabel = (LPCTSTR)label;

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileCommand::Compare(IEditorCommand * pOther)
{
   if (pOther == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEditorTerrainTileCommand> pTTCmd;
   if (pOther->QueryInterface(IID_IEditorTerrainTileCommand, (void**)&pTTCmd) == S_OK)
   {
      uint tile;
      ulong stamp;
      if ((pTTCmd->GetTile(&tile) == S_OK) && (pTTCmd->GetStamp(&stamp) == S_OK)
         && (tile == m_tile) && (stamp == m_stamp))
      {
         return S_OK;
      }
   }

   return S_FALSE;
}

////////////////////////////////////////

tResult cTerrainTileCommand::GetTile(uint * pTile)
{
   if (pTile == NULL)
   {
      return E_POINTER;
   }

   *pTile = m_tile;

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileCommand::GetStamp(ulong * pStamp)
{
   if (pStamp == NULL)
   {
      return E_POINTER;
   }

   *pStamp = m_stamp;

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
