///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYAPI_H
#define INCLUDED_ENTITYAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "matrix4.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEntity);
F_DECLARE_INTERFACE(IEntityManager);

class cRay;

F_DECLARE_INTERFACE(IDictionary);

class cAxisAlignedBox;


/////////////////////////////////////////////////////////////////////////////

class cTerrainLocatorHack
{
public:
   virtual void Locate(float nx, float nz, float * px, float * py, float * pz) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntity
//

interface IEntity : IUnknown
{
   virtual const tMatrix4 & GetWorldTransform() const = 0;

   virtual const cAxisAlignedBox & GetBoundingBox() const = 0;

   virtual void Update(double elapsedTime) = 0;
   virtual void Render() = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityManager
//

interface IEntityManager : IUnknown
{
   virtual void SetTerrainLocatorHack(cTerrainLocatorHack *) = 0;

   virtual tResult SpawnEntity(const tChar * pszMesh, float nx, float nz) = 0;
   virtual tResult SpawnEntity(const tChar * pszMesh, const tVec3 & position) = 0;

   virtual void RenderAll() = 0;

   virtual tResult GetEntityFromRayCast(const cRay & ray, IEntity * * ppEntity) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYAPI_H
