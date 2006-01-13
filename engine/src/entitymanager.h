////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "entityapi.h"

#include "model.h"
#include "saveloadapi.h"
#include "simapi.h"

#include "axisalignedbox.h"
#include "comenum.h"
#include "globalobjdef.h"
#include "techstring.h"

#include <list>
#include <set>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

typedef std::list<IEntity *> tEntityList;
typedef cComObject<cComEnum<IEntityEnum, &IID_IEntityEnum, IEntity*, CopyInterface<IEntity>, tEntityList>, &IID_IEntityEnum> tEntityListEnum;

////////////////////////////////////////////////////////////////////////////////

struct sLessInterface
{
   bool operator()(IUnknown * pLhs, IUnknown * pRhs) const
   {
      return !CTIsSameObject(pLhs, pRhs);
   }
};

typedef std::set<IEntity *, sLessInterface> tEntitySet;

typedef cComObject<cComEnum<IEntityEnum, &IID_IEntityEnum, IEntity*, CopyInterface<IEntity>, tEntitySet>, &IID_IEntityEnum> tEntitySetEnum;

template <>
void cComEnum<IEntityEnum, &IID_IEntityEnum, IEntity*, CopyInterface<IEntity>, tEntitySet>::Initialize(tEntitySet::const_iterator first,
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
// CLASS: cModelEntity
//

class cModelEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cModelEntity(tEntityId id, const tChar * pszModel, const tVec3 & position);
   ~cModelEntity();

   virtual tEntityId GetId() const;

   virtual uint GetFlags() const;
   virtual uint SetFlags(uint flags, uint mask);

   virtual const tMatrix4 & GetWorldTransform() const;

   virtual const tAxisAlignedBox & GetBoundingBox() const;

   virtual void Update(double elapsedTime);
   virtual void Render();

private:
   cStr m_model;
   cModel * m_pModel;
   tBlendedVertices m_blendedVerts;
   cAutoIPtr<IModelAnimationController> m_pAnimController;

   tEntityId m_id;
   uint m_flags;

   tVec3 m_position;

   tAxisAlignedBox m_bbox;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

class cEntityManager : public cComObject4<IMPLEMENTS(IEntityManager),
                                          IMPLEMENTS(IGlobalObject),
                                          IMPLEMENTS(ISimClient),
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

   virtual tResult SpawnEntity(const tChar * pszMesh, float nx, float nz);
   virtual tResult SpawnEntity(const tChar * pszMesh, const tVec3 & position);

   virtual tResult RemoveEntity(IEntity * pEntity);

   virtual void RenderAll();

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const;

   virtual tResult SelectBoxed(const tAxisAlignedBox & box);
   virtual tResult DeselectAll();
   virtual uint GetSelectedCount() const;
   virtual tResult GetSelected(IEntityEnum * * ppEnum) const;

   ///////////////////////////////////

   virtual void OnSimFrame(double elapsedTime);

   ///////////////////////////////////

   virtual tResult Save(IWriter * pWriter);
   virtual tResult Load(IReader * pReader, int version);

private:
   tEntityId m_nextId;
   tEntityList m_entities;
   tEntitySet m_selected;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
