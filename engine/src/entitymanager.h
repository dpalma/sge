///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMGR_H
#define INCLUDED_ENTITYMGR_H

#include "entityapi.h"

#include "globalobjdef.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ISceneEntity);

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

   virtual void SetTerrainLocatorHack(cTerrainLocatorHack *);

   virtual tResult SpawnEntity(const tChar * pszMesh, float x, float z);

   virtual void RenderAll();

private:
   ulong m_nextId;
   cTerrainLocatorHack * m_pTerrainLocatorHack;
   typedef std::list<ISceneEntity*> tEntities;
   tEntities m_entities;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMGR_H
