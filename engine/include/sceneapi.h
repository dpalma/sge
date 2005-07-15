///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENEAPI_H
#define INCLUDED_SCENEAPI_H

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;
template <typename T> class cVec3;
typedef class cVec3<float> tVec3;
template <typename T> class cMatrix4;
typedef cMatrix4<float> tMatrix4;
template <typename T> class cQuat;
typedef cQuat<float> tQuat;

F_DECLARE_INTERFACE(IRenderDevice);

class cRay;
F_DECLARE_INTERFACE(IScene);
F_DECLARE_INTERFACE(ISceneEntity);
F_DECLARE_INTERFACE(ISceneEntityEnum);
F_DECLARE_INTERFACE(ISceneCamera);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScene
//

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

   virtual tResult SetCamera(eSceneLayer layer, ISceneCamera * pCamera) = 0;
   virtual tResult GetCamera(eSceneLayer layer, ISceneCamera * * ppCamera) = 0;

   virtual void Clear(eSceneLayer layer) = 0;
   virtual void Clear() = 0;

   virtual tResult Query(const cRay & ray, ISceneEntityEnum * * ppEnum) = 0;

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

   virtual void Render(IRenderDevice * pRenderDevice) = 0;
   virtual float GetBoundingRadius() const = 0;

   virtual tResult Intersects(const cRay & ray) = 0;
};

///////////////////////////////////////

ENGINE_API ISceneEntity * SceneEntityCreate();
ENGINE_API ISceneEntity * SceneEntityCreate(const tChar * pszModel);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISceneEntityEnum
//

interface ISceneEntityEnum : IUnknown
{
   virtual tResult Next(ulong count, ISceneEntity * * ppEntities, ulong * pnEntities) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(ISceneEntityEnum * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISceneCamera
//

enum eProjectionType
{
   kPT_Perspective,
   kPT_Orthographic,
};

interface ISceneCamera : IUnknown
{
   virtual void SetPerspective(float fov, float aspect, float znear, float zfar) = 0;
   virtual void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar) = 0;

   virtual eProjectionType GetProjectionType() const = 0;

   virtual const tMatrix4 & GetViewMatrix() const = 0;
   virtual void SetViewMatrix(const tMatrix4 & view) = 0;

   virtual const tMatrix4 & GetProjectionMatrix() const = 0;
   virtual void SetProjectionMatrix(const tMatrix4 & projection) = 0;

   virtual const tMatrix4 & GetViewProjectionMatrix() const = 0;
   virtual const tMatrix4 & GetViewProjectionInverseMatrix() const = 0;

   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const = 0;
};

///////////////////////////////////////

ENGINE_API ISceneCamera * SceneCameraCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENEAPI_H
