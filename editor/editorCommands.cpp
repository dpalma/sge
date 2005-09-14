/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCommands.h"
#include "terrainapi.h"
#include "globalobj.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorCompositeCommand
//

////////////////////////////////////////

tResult EditorCompositeCommandCreate(IEditorCompositeCommand * * ppCommand)
{
   if (ppCommand == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEditorCompositeCommand> p(new cEditorCompositeCommand);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppCommand = CTAddRef(p);
   return S_OK;
}

////////////////////////////////////////

cEditorCompositeCommand::cEditorCompositeCommand()
{
}

////////////////////////////////////////

cEditorCompositeCommand::~cEditorCompositeCommand()
{
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      (*iter)->Release();
   }
   m_cmds.clear();
}

////////////////////////////////////////

tResult cEditorCompositeCommand::Do()
{
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      tResult result = (*iter)->Do();
      if (result != S_OK)
      {
         return result;
      }
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEditorCompositeCommand::CanUndo()
{
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      tResult result = (*iter)->CanUndo();
      if (result != S_OK)
      {
         return result;
      }
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEditorCompositeCommand::Undo()
{
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      tResult result = (*iter)->Undo();
      if (result != S_OK)
      {
         return result;
      }
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEditorCompositeCommand::GetLabel(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   if (m_cmds.empty())
   {
      return E_FAIL;
   }

   return m_cmds.front()->GetLabel(pLabel);
}

////////////////////////////////////////

tResult cEditorCompositeCommand::Add(IEditorCommand * pCommand)
{
   if (pCommand == NULL)
   {
      return E_POINTER;
   }

   m_cmds.push_back(CTAddRef(pCommand));
   return S_OK;
}

////////////////////////////////////////

tResult cEditorCompositeCommand::Remove(IEditorCommand * pCommand)
{
   if (pCommand == NULL)
   {
      return E_POINTER;
   }

   uint nRemoved = 0;
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      if (CTIsSameObject(*iter, pCommand))
      {
         (*iter)->Release();
         iter = m_cmds.erase(iter);
         nRemoved++;
      }
   }

   return nRemoved > 0 ? S_OK : S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

////////////////////////////////////////

cTerrainTileCommand::cTerrainTileCommand(uint ix, uint iz, uint tile)
 : m_ix(ix)
 , m_iz(iz)
 , m_tile(tile)
 , m_oldTile(tile)
{
}

////////////////////////////////////////

cTerrainTileCommand::~cTerrainTileCommand()
{
}

////////////////////////////////////////

tResult cTerrainTileCommand::Do()
{
   UseGlobal(TerrainModel);
   return pTerrainModel->SetQuadTile(m_ix, m_iz, m_tile, &m_oldTile);
}

////////////////////////////////////////

tResult cTerrainTileCommand::CanUndo()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileCommand::Undo()
{
   UseGlobal(TerrainModel);
   uint t;
   tResult result = pTerrainModel->SetQuadTile(m_ix, m_iz, m_oldTile, &t);
   Assert(t == m_tile);
   return result;
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

/////////////////////////////////////////////////////////////////////////////
