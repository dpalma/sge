///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "entityapi.h"

#include "model.h"
#include "saveloadapi.h"
#include "simapi.h"

#include "axisalignedbox.h"
#include "globalobjdef.h"
#include "techstring.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

typedef std::list<IEntity *> tEntities;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelEntity
//

class cModelEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cModelEntity(const tChar * pszModel, const tVec3 & position);
   ~cModelEntity();

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

   uint m_flags;

   tVec3 m_position;

   tAxisAlignedBox m_bbox;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


///////////////////////////////////////////////////////////////////////////////
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

   virtual void SetTerrainLocatorHack(cTerrainLocatorHack *);

   virtual tResult SpawnEntity(const tChar * pszMesh, float nx, float nz);
   virtual tResult SpawnEntity(const tChar * pszMesh, const tVec3 & position);

   virtual void RenderAll();

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const;

   ///////////////////////////////////

   virtual void OnSimFrame(double elapsedTime);

   ///////////////////////////////////

   virtual tResult Save(IWriter * pWriter);
   virtual tResult Load(IReader * pReader, int version);

private:
   ulong m_nextId;
   cTerrainLocatorHack * m_pTerrainLocatorHack;
   tEntities m_entities;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
