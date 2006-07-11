////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELAPI_H
#define INCLUDED_MODELAPI_H

#include "enginedll.h"
#include "comtools.h"

#include "renderapi.h"

#include "matrix4.h"
#include "quat.h"
#include "vec3.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IModelKeyFrameInterpolator);
F_DECLARE_INTERFACE(IModelAnimation);
F_DECLARE_INTERFACE(IModelAnimationController);
F_DECLARE_INTERFACE(IModelSkeleton);

struct sModelKeyFrame;

////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API cMatrix4<float>;
template class ENGINE_API std::allocator< cMatrix4<float> >;
template class ENGINE_API std::vector< cMatrix4<float> >;
#endif

typedef std::vector< cMatrix4<float> > tMatrices;


////////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelKeyFrame
//

////////////////////////////////////////

struct sModelKeyFrame
{
   double time;
   tVec3 translation;
   tQuat rotation;
};

////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API std::allocator<sModelKeyFrame>;
template class ENGINE_API std::vector<sModelKeyFrame>;
#endif
typedef std::vector<sModelKeyFrame> tModelKeyFrames;


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
   virtual tResult Advance(double elapsedTime) = 0;

   virtual const tMatrices & GetBlendMatrices() const = 0;

   virtual tResult SetAnimation(IModelAnimation * pAnim) = 0;
};

////////////////////////////////////////

ENGINE_API tResult ModelAnimationControllerCreate(IModelSkeleton * pSkeleton, IModelAnimationController * * ppAnimController);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModelSkeleton
//

struct sModelJoint
{
   int parentIndex;
   tMatrix4 localTransform;
   tVec3 localTranslation;
   tQuat localRotation;
};

typedef std::vector<sModelJoint> tModelJoints;

interface IModelSkeleton : IUnknown
{
   virtual tResult GetJointCount(size_t * pJointCount) const = 0;
   virtual tResult GetJoint(size_t iJoint, sModelJoint * pJoint) const = 0;

   virtual tResult GetBindMatrices(size_t nMaxMatrices, tMatrix4 * pMatrices) const = 0;

   virtual tResult AddAnimation(eModelAnimationType type, IModelAnimation * pAnim) = 0;
   virtual tResult GetAnimation(eModelAnimationType type, IModelAnimation * * ppAnim) const = 0;
};

ENGINE_API tResult ModelSkeletonCreate(const tModelJoints & joints, IModelSkeleton * * ppSkeleton);


////////////////////////////////////////////////////////////////////////////////
// 
// INTERFACE: IModel
//

struct sModelMeshInfo
{
   ePrimitiveType primitive;
   const uint16 * pIndices;
   uint nIndices;
   int materialIndex;
};

#define kRT_Model _T("Model") // resource type

interface IModel : IUnknown
{
   virtual tResult GetMeshCount(uint * pnMeshes) const = 0;
   virtual tResult GetMeshInfo(sModelMeshInfo * pMeshInfo) const = 0;

   virtual tResult GetMaterial() const = 0;

   virtual tResult GetVertices() const = 0;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELAPI_H
