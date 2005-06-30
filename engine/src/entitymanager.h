///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMGR_H
#define INCLUDED_ENTITYMGR_H

#include "entityapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

class cEntityManager : public cComObject2<IMPLEMENTS(IEntityManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cEntityManager();
   ~cEntityManager();

   DECLARE_NAME(EntityManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void SetRenderDeviceHack(IRenderDevice *);
   virtual void SetTerrainLocatorHack(cTerrainLocatorHack *);

   virtual tResult SpawnEntity(const char * pszMesh, float x, float z);

private:
   ulong m_nextId;
   IRenderDevice * m_pRenderDevice;
   cTerrainLocatorHack * m_pTerrainLocatorHack;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMGR_H
