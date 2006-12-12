///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorDocCmds.h"

#include "engine/entityapi.h"
#include "engine/saveloadapi.h"
#include "engine/terrainapi.h"

#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/readwriteapi.h"
#include "tech/techstring.h"


namespace ManagedEditor
{

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SelectEntityCommand
   //

   SelectEntityCommand::SelectEntityCommand(IEntity * pEntity)
    : m_pEntity(CTAddRef(pEntity))
   {
   }

   SelectEntityCommand::~SelectEntityCommand()
   {
   }

   SelectEntityCommand::!SelectEntityCommand()
   {
      if (m_pEntity != NULL)
      {
         m_pEntity->Release();
         m_pEntity = NULL;
      }
   }

   void SelectEntityCommand::Do()
   {
   }

   bool SelectEntityCommand::CanUndo()
   {
      return false;
   }

   void SelectEntityCommand::Undo()
   {
   }

   System::String ^ SelectEntityCommand::Label::get()
   {
      return "Select Entity";
   }


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: PlaceEntityCommand
   //

   PlaceEntityCommand::PlaceEntityCommand(System::String ^ entity, float nx, float nz)
    : m_entity(entity)
    , m_nx(nx)
    , m_nz(nz)
    , m_placedEntityId(0)
   {
   }

   void PlaceEntityCommand::Do()
   {
      if (m_placedEntityId != 0)
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
      return (m_placedEntityId != 0);
   }

   void PlaceEntityCommand::Undo()
   {
      if (m_placedEntityId == 0)
      {
         return;
      }

      UseGlobal(EntityManager);
//      pEntityManager->RemoveEntity(m_placedEntityId);
   }

   System::String ^ PlaceEntityCommand::Label::get()
   {
      return "Place Entity";
   }

} // namespace ManagedEditor
