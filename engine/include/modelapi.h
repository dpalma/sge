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

class cModel;
class cModelSkeleton;
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
};

////////////////////////////////////////

ENGINE_API tResult ModelAnimationControllerCreate(cModelSkeleton * pSkeleton, IModelAnimationController * * ppAnimController);


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

interface IModel : IUnknown
{
   virtual tResult GetMeshCount(uint * pnMeshes) const = 0;
   virtual tResult GetMeshInfo(sModelMeshInfo * pMeshInfo) const = 0;

   virtual tResult GetMaterial() const = 0;

   virtual tResult GetVertices() const = 0;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELAPI_H
