////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELAPI_H
#define INCLUDED_MODELAPI_H

#include "enginedll.h"

#include "tech/comtools.h"

#include "tech/matrix4.h"
#include "tech/matrix34.h"
#include "tech/quat.h"
#include "tech/vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IModelKeyFrameInterpolator);
F_DECLARE_INTERFACE(IModelAnimation);
F_DECLARE_INTERFACE(IModelAnimationController);
F_DECLARE_INTERFACE(IModelSkeleton);
F_DECLARE_INTERFACE(IModel);

struct sModelJoint;
struct sModelKeyFrame;
struct sModelMaterial;
struct sModelMesh;
struct sModelVertex;

////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API cMatrix4<float>;
template class ENGINE_API std::allocator< cMatrix4<float> >;
template class ENGINE_API std::vector< cMatrix4<float> >;
#endif


////////////////////////////////////////////////////////////////////////////////
//
// ENUM: eModelAnimationType
//

enum eModelAnimationType
{
   kMAT_Walk,
   kMAT_Run,
   kMAT_Death,
   kMAT_Attack,
   kMAT_Damage,
   kMAT_Idle,
};

struct sModelAnimationDesc
{
   eModelAnimationType type;
   uint start, end, fps;
};

ENGINE_API tResult AnimTypeFromString(const tChar * pszAnimTypeStr, eModelAnimationType * pAnimType);
ENGINE_API void ParseAnimDescs(const tChar * pszAnimString, std::vector<sModelAnimationDesc> * pAnimationDescs);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModelKeyFrameInterpolator
//

////////////////////////////////////////

enum eInterpolationMode
{
   kIM_Linear,
   kIM_Spline,
   kIM_Spherical, // for rotations only
};

////////////////////////////////////////

interface IModelKeyFrameInterpolator : IUnknown
{
   virtual tResult GetKeyFrameCount(uint * pnKeyFrames) const = 0;
   virtual tResult GetKeyFrame(uint index, sModelKeyFrame * pFrame) const = 0;

   virtual tResult GetInterpolationModes(eInterpolationMode * pTransInterpMode,
                                         eInterpolationMode * pRotInterpMode) const = 0;
   virtual tResult SetInterpolationModes(eInterpolationMode transInterpMode,
                                         eInterpolationMode rotInterpMode) = 0;

   virtual tResult Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult ModelKeyFrameInterpolatorCreate(const sModelKeyFrame * pKeyFrames, uint nKeyFrames,
                                                   IModelKeyFrameInterpolator * * ppInterp);



////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModelAnimation
//

interface IModelAnimation : IUnknown
{
   virtual tResult GetStartEnd(double * pStart, double * pEnd) const = 0;

   virtual tResult Interpolate(uint index, double time, tVec3 * pTrans, tQuat * pRot) const = 0;
};

////////////////////////////////////////

ENGINE_API tResult ModelAnimationCreate(IModelKeyFrameInterpolator * * pInterpolators,
                                        uint nInterpolators, IModelAnimation * * ppAnim);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModelAnimationController
//

interface IModelAnimationController : IUnknown
{
   virtual tResult Advance(double elapsedTime, uint nMaxMatrices, tMatrix34 * pMatrices) = 0;

   virtual tResult SetAnimation(IModelAnimation * pAnim) = 0;
};

////////////////////////////////////////

ENGINE_API tResult ModelAnimationControllerCreate(IModelSkeleton * pSkeleton, IModelAnimationController * * ppAnimController);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModelSkeleton
//

interface IModelSkeleton : IUnknown
{
   virtual tResult GetJointCount(size_t * pJointCount) const = 0;
   virtual tResult GetJoint(size_t iJoint, sModelJoint * pJoint) const = 0;

   virtual tResult GetBindMatrices(uint nMaxMatrices, tMatrix34 * pMatrices) const = 0;

   virtual tResult AddAnimation(eModelAnimationType type, IModelAnimation * pAnim) = 0;
   virtual tResult GetAnimation(eModelAnimationType type, IModelAnimation * * ppAnim) const = 0;
};

ENGINE_API tResult ModelSkeletonCreate(const sModelJoint * pJoints, uint nJoints, IModelSkeleton * * ppSkeleton);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModel
//

#define kRT_Model _T("Model") // resource type

interface IModel : IUnknown
{
   virtual tResult GetVertices(uint * pnVertices, const sModelVertex * * ppVertices) const = 0;

   virtual tResult GetIndices(uint * pnIndices, const uint16 * * ppIndices) const = 0;

   virtual tResult GetMeshes(uint * pnMeshes, const sModelMesh * * ppMeshes) const = 0;

   virtual tResult GetMaterialCount(uint * pnMaterials) const = 0;
   virtual tResult GetMaterial(uint index, sModelMaterial * pModelMaterial) const = 0;
   virtual const sModelMaterial * AccessMaterial(uint index) const = 0;

   virtual tResult GetSkeleton(IModelSkeleton * * ppSkeleton) = 0;
};

ENGINE_API tResult ModelCreateBox(const tVec3 & mins, const tVec3 & maxs, const float color[4], IModel * * ppModel);

ENGINE_API tResult ModelCreate(const sModelVertex * pVerts, size_t nVerts,
                               const uint16 * pIndices, size_t nIndices,
                               const sModelMesh * pMeshes, size_t nMeshes,
                               const sModelMaterial * pMaterials, size_t nMaterials,
                               IModelSkeleton * pSkeleton, IModel * * ppModel);

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELAPI_H
