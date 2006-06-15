///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entityapi.h"

#include "globalobj.h"
#include "multivar.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSpawn(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   if (pArgs == NULL)
   {
      return E_POINTER;
   }

   if (nArgs != 1 || !pArgs[0].IsString())
   {
      return E_INVALIDARG;
   }

   UseGlobal(EntityManager);
   pEntityManager->SpawnEntity(pArgs[0].ToString(), tVec3());
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSetRallyPoint(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltinEntityCommands()
{
   UseGlobal(EntityCommandManager);
   pEntityCommandManager->RegisterCommand(_T("Spawn"), EntityCommandSpawn);
   pEntityCommandManager->RegisterCommand(_T("SetRallyPoint"), EntityCommandSetRallyPoint);
}

///////////////////////////////////////////////////////////////////////////////
