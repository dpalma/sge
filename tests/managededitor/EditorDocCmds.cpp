///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorDocCmds.h"

#include "engine/entityapi.h"
#include "engine/terrainapi.h"

#include "tech/globalobj.h"
#include "tech/ray.h"
#include "tech/vec3.h"


namespace ManagedEditor
{

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SelectCommand
   //

   SelectCommand::SelectCommand()
    : m_rayOrigin(nullptr)
    , m_rayDirection(nullptr)
    , m_pOldSelection(NULL)
   {
   }

   SelectCommand::SelectCommand(XYZ<float> ^ rayOrigin, XYZ<float> ^ rayDirection)
    : m_rayOrigin(rayOrigin)
    , m_rayDirection(rayDirection)
    , m_pOldSelection(NULL)
   {
   }

   SelectCommand::~SelectCommand()
   {
   }

   SelectCommand::!SelectCommand()
   {
      if (m_pOldSelection != NULL)
      {
         m_pOldSelection->Release();
         m_pOldSelection = NULL;
      }
   }

   void SelectCommand::Do()
   {
      UseGlobal(EntityManager);

      Assert(m_pOldSelection == NULL);
      pin_ptr<IEnumEntities*> ppOldSelection = &m_pOldSelection;
      pEntityManager->GetSelected(ppOldSelection);
      pEntityManager->DeselectAll();

      if (m_rayOrigin && m_rayDirection)
      {
         cRay pickRay(
            tVec3(m_rayOrigin->X, m_rayOrigin->Y, m_rayOrigin->Z),
            tVec3(m_rayDirection->X, m_rayDirection->Y, m_rayDirection->Z));

         cAutoIPtr<IEntity> pEntity;
         if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
         {
            pEntityManager->Select(pEntity);
         }
      }
   }

   bool SelectCommand::CanUndo()
   {
      return true;
   }

   void SelectCommand::Undo()
   {
      UseGlobal(EntityManager);

      if (m_pOldSelection != NULL)
      {
         pEntityManager->SetSelected(m_pOldSelection);
         m_pOldSelection->Release();
         m_pOldSelection = NULL;
      }
      else
      {
         pEntityManager->DeselectAll();
      }
   }

   System::String ^ SelectCommand::Label::get()
   {
      return "Select";
   }


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: PlaceEntityCommand
   //

   PlaceEntityCommand::PlaceEntityCommand(System::String ^ entity, float nx, float nz)
    : m_entity(entity)
    , m_nx(nx)
    , m_nz(nz)
    , m_placedEntityId(kInvalidEntityId)
   {
   }

   void PlaceEntityCommand::Do()
   {
      if (m_placedEntityId != kInvalidEntityId)
      {
         return;
      }

      UseGlobal(EntityManager);
      UseGlobal(TerrainModel);

      tVec3 loc;
      if (pTerrainModel->GetPointOnTerrain(m_nx, m_nz, &loc) == S_OK)
      {
         pin_ptr<ulong> pEntityId = &m_placedEntityId;
         pEntityManager->SpawnEntity("zombie", loc, pEntityId);
      }
   }

   bool PlaceEntityCommand::CanUndo()
   {
      return (m_placedEntityId != kInvalidEntityId);
   }

   void PlaceEntityCommand::Undo()
   {
      if (m_placedEntityId == kInvalidEntityId)
      {
         return;
      }

      UseGlobal(EntityManager);
      pEntityManager->RemoveEntity(m_placedEntityId);
      m_placedEntityId = kInvalidEntityId; // When the entity is removed, the id is no longer valid
   }

   System::String ^ PlaceEntityCommand::Label::get()
   {
      return "Place Entity";
   }


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: ChangeTerrainElevationCommand
   //

   ChangeTerrainElevationCommand::ChangeTerrainElevationCommand(HTERRAINVERTEX hVertex, float elevDelta)
    : m_hVertex(hVertex)
    , m_elevDelta(elevDelta)
    , m_oldElevation(0)
   {
   }

   void ChangeTerrainElevationCommand::Do()
   {
      if (m_hVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainModel);

         tVec3 vertexPos;
         if (pTerrainModel->GetVertexPosition(m_hVertex, &vertexPos) == S_OK)
         {
            m_oldElevation = vertexPos.y;
         }

         pTerrainModel->ChangeVertexElevation(m_hVertex, m_elevDelta);
      }
   }

   bool ChangeTerrainElevationCommand::CanUndo()
   {
      return (m_hVertex != INVALID_HTERRAINVERTEX);
   }

   void ChangeTerrainElevationCommand::Undo()
   {
      if (m_hVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainModel);
         pTerrainModel->SetVertexElevation(m_hVertex, m_oldElevation);
      }
   }

   System::String ^ ChangeTerrainElevationCommand::Label::get()
   {
      return "Terrain Elevation";
   }


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SetTerrainElevationCommand
   //

   SetTerrainElevationCommand::SetTerrainElevationCommand(HTERRAINVERTEX hVertex, float elevation)
    : m_hVertex(hVertex)
    , m_elevation(elevation)
    , m_oldElevation(elevation)
   {
   }

   void SetTerrainElevationCommand::Do()
   {
      if (m_hVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainModel);

         tVec3 vertexPos;
         if (pTerrainModel->GetVertexPosition(m_hVertex, &vertexPos) == S_OK)
         {
            m_oldElevation = vertexPos.y;
         }

         pTerrainModel->SetVertexElevation(m_hVertex, m_elevation);
      }
   }

   bool SetTerrainElevationCommand::CanUndo()
   {
      return true;
   }

   void SetTerrainElevationCommand::Undo()
   {
      if (m_hVertex != INVALID_HTERRAINVERTEX)
      {
         UseGlobal(TerrainModel);
         pTerrainModel->SetVertexElevation(m_hVertex, m_oldElevation);
      }
   }

   System::String ^ SetTerrainElevationCommand::Label::get()
   {
      return "Terrain Plateau";
   }


} // namespace ManagedEditor
