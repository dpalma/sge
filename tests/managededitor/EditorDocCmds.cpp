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
   // CLASS: PlaceEntityCommand
   //

   PlaceEntityCommand::PlaceEntityCommand(System::String ^ entity, float x, float z)
    : m_entity(entity)
    , m_x(x)
    , m_z(z)
   {
   }

   PlaceEntityCommand::~PlaceEntityCommand()
   {
   }

   void PlaceEntityCommand::Do()
   {
   }

   bool PlaceEntityCommand::CanUndo()
   {
      return false;
   }

   void PlaceEntityCommand::Undo()
   {
   }

   System::String ^ PlaceEntityCommand::GetLabel()
   {
      return nullptr;
   }

} // namespace ManagedEditor
