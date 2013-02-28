/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorCommands.h"

#include "engine/entityapi.h"
#include "engine/terrainapi.h"

#include "tech/globalobj.h"

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
   return EditorCompositeCommandCreate(NULL, ppCommand);
}

////////////////////////////////////////

tResult EditorCompositeCommandCreate(tEditorCompositeCommandCallback pfnCallback,
                                     IEditorCompositeCommand * * ppCommand)
{
   if (ppCommand == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEditorCompositeCommand> p(new cEditorCompositeCommand(pfnCallback));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppCommand = CTAddRef(p);
   return S_OK;
}

////////////////////////////////////////

cEditorCompositeCommand::cEditorCompositeCommand(tEditorCompositeCommandCallback pfnCallback)
 : m_pfnCallback(pfnCallback)
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

   m_pfnCallback = NULL;
}

////////////////////////////////////////

tResult cEditorCompositeCommand::Do()
{
   DoCallback(kPreDo);
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      tResult result = (*iter)->Do();
      if (result != S_OK)
      {
         DoCallback(kPostDo);
         return result;
      }
   }
   DoCallback(kPostDo);
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
   DoCallback(kPreUndo);
   tCmds::iterator iter = m_cmds.begin();
   tCmds::iterator end = m_cmds.end();
   for (; iter != end; iter++)
   {
      tResult result = (*iter)->Undo();
      if (result != S_OK)
      {
         DoCallback(kPostUndo);
         return result;
      }
   }
   DoCallback(kPostUndo);
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

////////////////////////////////////////

void cEditorCompositeCommand::DoCallback(eEditorCompositeCommandCallback type)
{
   if (m_pfnCallback != NULL)
   {
      (*m_pfnCallback)(type);
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

////////////////////////////////////////

cTerrainTileCommand::cTerrainTileCommand(HTERRAINQUAD hQuad, uint tile)
 : m_hQuad(hQuad)
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
   if (m_hQuad != INVALID_HTERRAINQUAD)
   {
      pTerrainModel->GetQuadTile(m_hQuad, &m_oldTile);
      return pTerrainModel->SetQuadTile(m_hQuad, m_tile);
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
   UseGlobal(TerrainModel);
   if (m_hQuad != INVALID_HTERRAINQUAD)
   {
      uint tile;
      pTerrainModel->GetQuadTile(m_hQuad, &tile);
      tResult result = pTerrainModel->SetQuadTile(m_hQuad, m_oldTile);
      Assert(tile == m_tile);
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


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChangeElevationCommand
//

////////////////////////////////////////

cTerrainChangeElevationCommand::cTerrainChangeElevationCommand(HTERRAINVERTEX hVertex, float elevDelta)
 : m_hVertex(hVertex)
 , m_elevDelta(elevDelta)
 , m_oldElevation(0)
{
}

////////////////////////////////////////

cTerrainChangeElevationCommand::~cTerrainChangeElevationCommand()
{
}

////////////////////////////////////////

tResult cTerrainChangeElevationCommand::Do()
{
   if (m_hVertex != INVALID_HTERRAINVERTEX)
   {
      UseGlobal(TerrainModel);

      tVec3 vertexPos;
      if (pTerrainModel->GetVertexPosition(m_hVertex, &vertexPos) == S_OK)
      {
         m_oldElevation = vertexPos.y;
      }

      return pTerrainModel->ChangeVertexElevation(m_hVertex, m_elevDelta);
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainChangeElevationCommand::CanUndo()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainChangeElevationCommand::Undo()
{
   if (m_hVertex != INVALID_HTERRAINVERTEX)
   {
      UseGlobal(TerrainModel);
      return pTerrainModel->SetVertexElevation(m_hVertex, m_oldElevation);
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainChangeElevationCommand::GetLabel(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   CString label;
   if (!label.LoadString(IDS_TERRAIN_CHANGE_ELEV_COMMAND_LABEL))
   {
      return E_FAIL;
   }

   *pLabel = (LPCTSTR)label;

   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainSetElevationCommand
//

////////////////////////////////////////

cTerrainSetElevationCommand::cTerrainSetElevationCommand(HTERRAINVERTEX hVertex, float elevation, uint labelId)
 : m_hVertex(hVertex)
 , m_elevation(elevation)
 , m_oldElevation(elevation)
 , m_labelId(labelId)
{
}

////////////////////////////////////////

cTerrainSetElevationCommand::~cTerrainSetElevationCommand()
{
}

////////////////////////////////////////

tResult cTerrainSetElevationCommand::Do()
{
   if (m_hVertex != INVALID_HTERRAINVERTEX)
   {
      UseGlobal(TerrainModel);

      tVec3 vertexPos;
      if (pTerrainModel->GetVertexPosition(m_hVertex, &vertexPos) == S_OK)
      {
         m_oldElevation = vertexPos.y;
      }

      return pTerrainModel->SetVertexElevation(m_hVertex, m_elevation);
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainSetElevationCommand::CanUndo()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainSetElevationCommand::Undo()
{
   if (m_hVertex != INVALID_HTERRAINVERTEX)
   {
      UseGlobal(TerrainModel);
      return pTerrainModel->SetVertexElevation(m_hVertex, m_oldElevation);
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cTerrainSetElevationCommand::GetLabel(cStr * pLabel)
{
   if (pLabel == NULL)
   {
      return E_POINTER;
   }

   CString label;
   if (!label.LoadString(m_labelId))
   {
      return E_FAIL;
   }

   *pLabel = (LPCTSTR)label;

   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlaceEntityCommand
//

////////////////////////////////////////

cPlaceEntityCommand::cPlaceEntityCommand(const tChar * pszEntity, const tVec3 & position)
 : m_entity(pszEntity)
 , m_position(position)
{
}

////////////////////////////////////////

cPlaceEntityCommand::~cPlaceEntityCommand()
{
}

////////////////////////////////////////

tResult cPlaceEntityCommand::Do()
{
   if (!m_entity.empty())
   {
      UseGlobal(EntityManager);
      pEntityManager->SpawnEntity(m_entity.c_str(), m_position);
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cPlaceEntityCommand::CanUndo()
{
   return S_FALSE;
}

////////////////////////////////////////

tResult cPlaceEntityCommand::Undo()
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cPlaceEntityCommand::GetLabel(cStr * pLabel)
{
   return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////////////////
