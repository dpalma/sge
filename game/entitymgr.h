///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMGR_H
#define INCLUDED_ENTITYMGR_H

#include "entityapi.h"

#include "globalobj.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

class cEntityManager : public cGlobalObject<IMPLEMENTS(IEntityManager)>
{
public:
   cEntityManager();
   ~cEntityManager();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SpawnEntity(const char * pszMesh, const tVec3 & location);

private:
   ulong m_nextId;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMGR_H
