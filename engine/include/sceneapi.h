///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEAPI_H
#define INCLUDED_SCENEAPI_H

#include "enginedll.h"
#include "comtools.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

class cRay;
template <typename T> class cVec3;
typedef class cVec3<float> tVec3;
template <typename T> class cMatrix4;
typedef cMatrix4<float> tMatrix4;
typedef class cQuat tQuat;

F_DECLARE_INTERFACE(IRenderDevice);

F_DECLARE_INTERFACE(IScene);
F_DECLARE_INTERFACE(ISceneEntity);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScene
//

typedef std::list<ISceneEntity *> tSceneEntityList;

enum eSceneLayer
{
   kSL_Terrain,
   kSL_Object,
   kSL_InGameUI,
   kSL_FogOfWar,
   kMAXSCENELAYERS
};

interface IScene : IUnknown
{
   virtual tResult AddEntity(eSceneLayer layer, ISceneEntity * pEntity) = 0;
   virtual tResult RemoveEntity(eSceneLayer layer, ISceneEntity * pEntity) = 0;

   virtual void Clear(eSceneLayer layer) = 0;
   virtual void Clear() = 0;

   virtual tResult Query(const cRay & ray, tSceneEntityList * pEntities) = 0;

   virtual tResult Render(IRenderDevice * pRenderDevice) = 0;
};

///////////////////////////////////////

ENGINE_API void SceneCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISceneEntity
//

interface ISceneEntity : IUnknown
{
   // Parent-child relationships govern world position only, not rendering order.
   virtual ISceneEntity * AccessParent() = 0;
   virtual tResult SetParent(ISceneEntity * pEntity) = 0;
   virtual tResult IsChild(ISceneEntity * pEntity) const = 0;
   virtual tResult AddChild(ISceneEntity * pEntity) = 0;
   virtual tResult RemoveChild(ISceneEntity * pEntity) = 0;

   virtual const tVec3 & GetLocalTranslation() const = 0;
   virtual void SetLocalTranslation(const tVec3 & translation) = 0;
   virtual const tQuat & GetLocalRotation() const = 0;
   virtual void SetLocalRotation(const tQuat & rotation) = 0;
   virtual const tMatrix4 & GetLocalTransform() const = 0;

   virtual const tVec3 & GetWorldTranslation() const = 0;
   virtual const tQuat & GetWorldRotation() const = 0;
   virtual const tMatrix4 & GetWorldTransform() const = 0;

   virtual void Render() = 0;
   virtual float GetBoundingRadius() const = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEAPI_H
