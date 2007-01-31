////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "engine/entityapi.h"

#include "platform/inputapi.h"
#include "engine/saveloadapi.h"

#include "tech/axisalignedbox.h"
#include "tech/comenum.h"
#include "tech/connptimpl.h"
#include "tech/globalobjdef.h"
#include "tech/simapi.h"

#include <list>
#include <map>
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

typedef std::set<IEntity *, cCTLessInterface> tEntitySet;

typedef cComObject<cComEnum<IEnumEntities, &IID_IEnumEntities, IEntity*, CopyInterface<IEntity>, tEntitySet>, &IID_IEnumEntities> tEntitySetEnum;

template <>
void cComEnum<IEnumEntities, &IID_IEnumEntities, IEntity*, CopyInterface<IEntity>, tEntitySet>::Initialize(tEntitySet::const_iterator first,
                                                                                                           tEntitySet::const_iterator last)
{
   tEntitySet::const_iterator iter = first;
   for (; iter != last; iter++)
   {
      IEntity * t;
      CopyInterface<IEntity>::Copy(&t, &(*iter));
      m_elements.insert(t);
   }
   m_iterator = m_elements.begin();
}


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

   virtual tResult RemoveEntity(tEntityId entityId);
   virtual tResult RemoveEntity(IEntity * pEntity);
   virtual void RemoveAll();

   virtual void RenderAll();

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEnumEntities * * ppEnum) const;

   virtual tResult Select(IEntity * pEntity);
   virtual tResult SelectBoxed(const tAxisAlignedBox & box);
   virtual tResult DeselectAll();
   virtual uint GetSelectedCount() const;
   virtual tResult SetSelected(IEnumEntities * pEnum);
   virtual tResult GetSelected(IEnumEntities * * ppEnum) const;

   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            tEntityComponentFactoryFn pfnFactory, void * pUser);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

   ///////////////////////////////////

   void RegisterEntityUpdatables(IEntity * pEntity);
   void RevokeEntityUpdatables(IEntity * pEntity);

   ///////////////////////////////////

   virtual tResult Save(IWriter * pWriter);
   virtual tResult Load(IReader * pReader, int version);
   virtual void Reset();

private:
   bool IsSelected(IEntity * pEntity) const;

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

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
   public:
      virtual void DeleteThis() {}
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };
   friend class cInputListener;
   cInputListener m_inputListener;

   tEntityId m_nextId;
   tEntityList m_entities;
   tEntitySet m_selected;

   typedef std::map<cStr, std::pair<tEntityComponentFactoryFn, void*> > tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
