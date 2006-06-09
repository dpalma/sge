////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "entityapi.h"

#include "inputapi.h"
#include "saveloadapi.h"
#include "schedulerapi.h"

#include "axisalignedbox.h"
#include "comenum.h"
#include "connptimpl.h"
#include "globalobjdef.h"

#include <list>
#include <map>
#include <set>

#ifdef _MSC_VER
#pragma once
#endif

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
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   ///////////////////////////////////

   virtual tResult AddEntityManagerListener(IEntityManagerListener * pListener);
   virtual tResult RemoveEntityManagerListener(IEntityManagerListener * pListener);

   virtual tResult SpawnEntity(const tChar * pszEntity, const tVec3 & position);

   virtual tResult RemoveEntity(IEntity * pEntity);
   virtual void RemoveAll();

   virtual void RenderAll();

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEnumEntities * * ppEnum) const;

   virtual tResult Select(IEntity * pEntity);
   virtual tResult SelectBoxed(const tAxisAlignedBox & box);
   virtual tResult DeselectAll();
   virtual uint GetSelectedCount() const;
   virtual tResult GetSelected(IEnumEntities * * ppEnum) const;

   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            tEntityComponentFactoryFn pfnFactory);
   virtual tResult RevokeComponentFactory(const tChar * pszComponent);
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent);

   ///////////////////////////////////

   void SimFrame(double elapsedTime);

   ///////////////////////////////////

   virtual tResult Save(IWriter * pWriter);
   virtual tResult Load(IReader * pReader, int version);
   virtual void Reset();

private:
   bool IsSelected(IEntity * pEntity) const;

   class cAnimateTask : public cComObject<IMPLEMENTS(ITask)>
   {
   public:
      cAnimateTask(cEntityManager * pOuter);
      virtual void DeleteThis() {}
      virtual void Execute(double time);
   private:
      cEntityManager * m_pOuter;
      double m_lastTime;
   };
   friend class cAnimateTask;
   cAnimateTask m_animateTask;

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

   typedef std::map<cStr, tEntityComponentFactoryFn> tComponentFactoryMap;
   tComponentFactoryMap m_componentFactoryMap;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
