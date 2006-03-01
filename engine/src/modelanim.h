///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELANIM_H
#define INCLUDED_MODELANIM_H

#include "enginedll.h"
#include "modelapi.h"

#include "quat.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

class cModel;
class cModelSkeleton;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelKeyFrameInterpolator
//

class cModelKeyFrameInterpolator : public cComObject<IMPLEMENTS(IModelKeyFrameInterpolator)>
{
   cModelKeyFrameInterpolator(const cModelKeyFrameInterpolator & other);
   void operator =(const cModelKeyFrameInterpolator & other);

public:
   cModelKeyFrameInterpolator();
   cModelKeyFrameInterpolator(const sModelKeyFrame * pKeyFrames, uint nKeyFrames);
   ~cModelKeyFrameInterpolator();

   virtual tResult GetKeyFrameCount(uint * pnKeyFrames) const;
   virtual tResult GetKeyFrame(uint index, sModelKeyFrame * pFrame) const;

   virtual tResult GetInterpolationModes(eInterpolationMode * pTransInterpMode,
                                         eInterpolationMode * pRotInterpMode) const;
   virtual tResult SetInterpolationModes(eInterpolationMode transInterpMode,
                                         eInterpolationMode rotInterpMode);

   virtual tResult Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const;

private:
   std::vector<sModelKeyFrame> m_keyFrames;
   eInterpolationMode m_transInterpMode, m_rotInterpMode;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimation
//

class cModelAnimation : public cComObject<IMPLEMENTS(IModelAnimation)>
{
   cModelAnimation(const cModelAnimation & other);
   void operator =(const cModelAnimation & other);

public:
   cModelAnimation(IModelKeyFrameInterpolator * * pInterpolators, uint nInterpolators);
   ~cModelAnimation();

   virtual tResult GetStartEnd(double * pStart, double * pEnd) const;

   virtual tResult Interpolate(uint index, double time, tVec3 * pTrans, tQuat * pRot) const;

private:
   std::vector<IModelKeyFrameInterpolator*> m_interpolators;
   double m_start, m_end;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimationController
//

class cModelAnimationController : public cComObject<IMPLEMENTS(IModelAnimationController)>
{
   cModelAnimationController(const cModelAnimationController & other);
   void operator =(const cModelAnimationController & other);

public:
   cModelAnimationController(cModelSkeleton * pSkeleton, IModelAnimation * pAnim);
   ~cModelAnimationController();

   tResult Advance(double elapsedTime);

   const tMatrices & GetBlendMatrices() const { return m_blendMatrices; }

private:
   cModelSkeleton * m_pSkeleton;
   cAutoIPtr<IModelAnimation> m_pAnim;
   double m_animStart, m_animEnd, m_animTime;
   tMatrices m_blendMatrices;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELANIM_H
