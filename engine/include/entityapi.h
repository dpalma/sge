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
F_DECLARE_INTERFACE(IEntityEnum);
F_DECLARE_INTERFACE(IEntityManager);

class cRay;

F_DECLARE_INTERFACE(IDictionary);

template <typename T> class cAxisAlignedBox;
typedef class cAxisAlignedBox<float> tAxisAlignedBox;


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

////////////////////////////////////////

enum eEntityFlags
{
   kEF_None                   = 0,
   kEF_Selected               = (1 << 0),
   kEF_All                    = 0xFFFFFFFF,
};

////////////////////////////////////////

interface IEntity : IUnknown
{
   virtual uint GetFlags() const = 0;
   virtual uint SetFlags(uint flags, uint mask) = 0;

   virtual const tMatrix4 & GetWorldTransform() const = 0;

   virtual const tAxisAlignedBox & GetBoundingBox() const = 0;

   virtual void Update(double elapsedTime) = 0;
   virtual void Render() = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityEnum
//

interface IEntityEnum : IUnknown
{
   virtual tResult Next(ulong count, IEntity * * ppEntities, ulong * pnEntities) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEntityEnum * * ppEnum) = 0;
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

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const = 0;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityManagerCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYAPI_H
