////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "engine/entityapi.h"

#include "engine/saveloadapi.h"

#include "tech/axisalignedbox.h"
#include "tech/comenum.h"
#include "tech/connptimpl.h"
#include "tech/globalobjdef.h"
#include "tech/simapi.h"

#include <list>
#include <set>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

typedef std::set<IUpdatable *, cCTLessInterface> tUpdatableSet;
typedef std::list<IUpdatable *> tUpdatableList;

////////////////////////////////////////////////////////////////////////////////

typedef std::list<IEntity *> tEntityList;

typedef cComObject<cComEnum<IEnumEntities, &IID_IEnumEntities, IEntity*, CopyInterface<IEntity>, tEntityList>, &IID_IEnumEntities> tEntityListEnum;


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

class cEntityManager : public cComObject3<IMPLEMENTSCP(IEntityManager, IEntityManagerListener),
                                          IMPLEMENTS(IGlobalObject),
                                          IMPLEMENTS(ISaveLoadParticipant)>
{
public:
   cEntityManager();
   ~cEntityManager();

   DECLARE_NAME(EntityManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   ///////////////////////////////////

   virtual tResult AddEntityManagerListener(IEntityManagerListener * pListener);
   virtual tResult RemoveEntityManagerListener(IEntityManagerListener * pListener);

   virtual tResult SpawnEntity(const tChar * pszEntity, const tVec3 & position, tEntityId * pEntityId);

   tResult AddEntity(IEntity * pEntity);

   virtual tResult RemoveEntity(tEntityId entityId);
   virtual tResult RemoveEntity(IEntity * pEntity);
   virtual void RemoveAll();

   virtual void RenderAll();

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEnumEntities * * ppEnum) const;

   ///////////////////////////////////

   void RegisterEntityUpdatables(IEntity * pEntity);
   void RevokeEntityUpdatables(IEntity * pEntity);

   ///////////////////////////////////

   virtual tResult Save(IWriter * pWriter);
   virtual tResult Load(IReader * pReader, int version);
   virtual void Reset();

private:
   class cSimClient : public cComObject<IMPLEMENTS(ISimClient)>
   {
   public:
      cSimClient();
      virtual void DeleteThis() {}
      virtual tResult Execute(double time);
      tResult AddUpdatable(IUpdatable * pUpdatable);
      tResult RemoveUpdatable(IUpdatable * pUpdatable);
      void RemoveAll();
   private:
      double m_lastTime;
      tUpdatableList m_updatables;
   };
   friend class cSimClient;
   cSimClient m_simClient;

   tEntityList m_entities;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
