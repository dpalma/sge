///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYMANAGER_H
#define INCLUDED_ENTITYMANAGER_H

#include "entityapi.h"

#include "model.h"
#include "simapi.h"

#include "globalobjdef.h"
#include "matrix4.h"
#include "techstring.h"
#include "vec3.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelEntity
//

class cModelEntity : public cComObject<IMPLEMENTS(IUnknown)>
{
public:
   cModelEntity(const tChar * pszModel, const tVec3 & position);
   ~cModelEntity();

   const tMatrix4 & GetWorldTransform() const;

   void Update(double elapsedTime);
   void Render();

private:
   cStr m_model;
   cModel * m_pModel;
   double m_animationTime, m_animationLength;
   tBlendedVertices m_blendedVerts;
   std::vector<tMatrix4> m_blendMatrices;

   tVec3 m_position;

   mutable bool m_bUpdateWorldTransform;
   mutable tMatrix4 m_worldTransform;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

class cEntityManager : public cComObject3<IMPLEMENTS(IEntityManager),
                                          IMPLEMENTS(IGlobalObject),
                                          IMPLEMENTS(ISimClient)>
{
public:
   cEntityManager();
   ~cEntityManager();

   DECLARE_NAME(EntityManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void SetTerrainLocatorHack(cTerrainLocatorHack *);

   virtual tResult SpawnEntity(const tChar * pszMesh, float nx, float nz);
   tResult SpawnEntity(const tChar * pszMesh, const tVec3 & position);

   virtual void RenderAll();

   virtual void OnSimFrame(double elapsedTime);

private:
   ulong m_nextId;
   cTerrainLocatorHack * m_pTerrainLocatorHack;
   typedef std::list<cModelEntity*> tEntities;
   tEntities m_entities;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYMANAGER_H
