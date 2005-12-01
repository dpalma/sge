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
// CLASS: cModelAnimationController
//

class cModelAnimationController : public cComObject<IMPLEMENTS(IModelAnimationController)>
{
   cModelAnimationController(const cModelAnimationController & other);
   void operator =(const cModelAnimationController & other);

public:
   cModelAnimationController(cModel * pModel);
   ~cModelAnimationController();

   tResult GetModel(cModel * * ppModel) const;
   tResult SetModel(cModel * pModel);

   tResult Advance(double elapsedTime);

   const tMatrices & GetBlendMatrices() const { return m_blendMatrices; }

private:
   cModel * m_pModel;
   double m_animTime, m_animLength;
   tMatrices m_blendMatrices;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELANIM_H
