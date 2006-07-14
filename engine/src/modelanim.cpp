///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelanim.h"
#include "model.h"

#include "matrix3.h"
#include "matrix4.h"

#include <algorithm>
#include <cfloat>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelKeyFrameInterpolator
//

///////////////////////////////////////

cModelKeyFrameInterpolator::cModelKeyFrameInterpolator()
 : m_transInterpMode(kIM_Linear)
 , m_rotInterpMode(kIM_Spherical)
{
}

///////////////////////////////////////

cModelKeyFrameInterpolator::cModelKeyFrameInterpolator(const sModelKeyFrame * pKeyFrames, uint nKeyFrames)
 : m_keyFrames(nKeyFrames)
 , m_transInterpMode(kIM_Linear)
 , m_rotInterpMode(kIM_Spherical)
{
   for (uint i = 0; i < nKeyFrames; i++)
   {
      m_keyFrames[i] = pKeyFrames[i];
   }
}

///////////////////////////////////////

cModelKeyFrameInterpolator::~cModelKeyFrameInterpolator()
{
}

///////////////////////////////////////

tResult cModelKeyFrameInterpolator::GetKeyFrameCount(uint * pnKeyFrames) const
{
   if (pnKeyFrames == NULL)
   {
      return E_POINTER;
   }
   *pnKeyFrames = m_keyFrames.size();
   return S_OK;
}

///////////////////////////////////////

tResult cModelKeyFrameInterpolator::GetKeyFrame(uint index, sModelKeyFrame * pFrame) const
{
   if (pFrame == NULL)
   {
      return E_POINTER;
   }

   if (index >= m_keyFrames.size())
   {
      return E_INVALIDARG;
   }

   *pFrame = m_keyFrames[index];
   return S_OK;
}

///////////////////////////////////////

tResult cModelKeyFrameInterpolator::GetInterpolationModes(eInterpolationMode * pTransInterpMode,
                                                          eInterpolationMode * pRotInterpMode) const
{
   if (pTransInterpMode == NULL || pRotInterpMode == NULL)
   {
      return E_POINTER;
   }
   *pTransInterpMode = m_transInterpMode;
   *pRotInterpMode = m_rotInterpMode;
   return S_OK;
}

///////////////////////////////////////

tResult cModelKeyFrameInterpolator::SetInterpolationModes(eInterpolationMode transInterpMode,
                                                          eInterpolationMode rotInterpMode)
{
   if (transInterpMode == kIM_Spherical || rotInterpMode == kIM_Spline)
   {
      return E_INVALIDARG;
   }
   m_transInterpMode = transInterpMode;
   m_rotInterpMode = rotInterpMode;
   return S_OK;
}

///////////////////////////////////////

tResult cModelKeyFrameInterpolator::Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const
{
   if (pTrans == NULL || pRot == NULL)
   {
      return E_POINTER;
   }

   tModelKeyFrames::const_iterator iter = m_keyFrames.begin();
   tModelKeyFrames::const_iterator end = m_keyFrames.end();
   tModelKeyFrames::const_iterator prev = iter;
   for (; iter != end; prev = iter, iter++)
   {
      if (iter->time >= time)
      {
         if (iter == prev)
         {
            *pRot = iter->rotation;
            *pTrans = iter->translation;
         }
         else
         {
            double u = (time - prev->time) / (iter->time - prev->time);
            *pRot = QuatSlerp(prev->rotation, iter->rotation, static_cast<float>(u));
            *pTrans = Vec3Lerp(prev->translation, iter->translation, (tVec3::value_type)u);
         }

         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult ModelKeyFrameInterpolatorCreate(const sModelKeyFrame * pKeyFrames, uint nKeyFrames,
                                        IModelKeyFrameInterpolator * * ppInterp)
{
   if (pKeyFrames == NULL || ppInterp == NULL)
   {
      return E_POINTER;
   }

   if (nKeyFrames == 0)
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<IModelKeyFrameInterpolator> pInterp;
   pInterp = static_cast<IModelKeyFrameInterpolator*>(new cModelKeyFrameInterpolator(pKeyFrames, nKeyFrames));
   if (!pInterp)
   {
      return E_OUTOFMEMORY;
   }
   *ppInterp = CTAddRef(pInterp);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimation
//

///////////////////////////////////////

cModelAnimation::cModelAnimation(IModelKeyFrameInterpolator * * pInterpolators,
                                 uint nInterpolators)
 : m_interpolators(nInterpolators)
 , m_start(DBL_MAX)
 , m_end(DBL_MIN)
{
   for (uint i = 0; i < nInterpolators; i++)
   {
      m_interpolators[i] = CTAddRef(pInterpolators[i]);

      uint nKeyFrames = 0;
      sModelKeyFrame startFrame, endFrame;
      if (m_interpolators[i]->GetKeyFrameCount(&nKeyFrames) == S_OK
         && m_interpolators[i]->GetKeyFrame(0, &startFrame) == S_OK
         && m_interpolators[i]->GetKeyFrame(nKeyFrames - 1, &endFrame) == S_OK)
      {
         if (startFrame.time < m_start)
         {
            m_start = startFrame.time;
         }
         if (endFrame.time > m_end)
         {
            m_end = endFrame.time;
         }
      }
   }
}

///////////////////////////////////////

cModelAnimation::~cModelAnimation()
{
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethod(&IUnknown::Release));
   m_interpolators.clear();
}

///////////////////////////////////////

tResult cModelAnimation::GetStartEnd(double * pStart, double * pEnd) const
{
   if (pStart == NULL || pEnd == NULL)
   {
      return E_POINTER;
   }
   *pStart = m_start;
   *pEnd = m_end;
   return S_OK;
}

///////////////////////////////////////

tResult cModelAnimation::Interpolate(uint index, double time, tVec3 * pTrans, tQuat * pRot) const
{
   if (index >= m_interpolators.size())
   {
      return E_INVALIDARG;
   }
   if (time < m_start || time > m_end)
   {
      return E_INVALIDARG;
   }
   return m_interpolators[index]->Interpolate(time, pTrans, pRot);
}

///////////////////////////////////////

tResult ModelAnimationCreate(IModelKeyFrameInterpolator * * pInterpolators,
                             uint nInterpolators, IModelAnimation * * ppAnim)
{
   if (pInterpolators == NULL || ppAnim == NULL)
   {
      return E_POINTER;
   }
   if (nInterpolators == 0)
   {
      return E_INVALIDARG;
   }
   cAutoIPtr<IModelAnimation> pAnim;
   pAnim = static_cast<IModelAnimation*>(new cModelAnimation(pInterpolators, nInterpolators));
   if (!pAnim)
   {
      return E_OUTOFMEMORY;
   }
   *ppAnim = CTAddRef(pAnim);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimationController
//

///////////////////////////////////////

cModelAnimationController::cModelAnimationController(IModelSkeleton * pSkeleton)
 : m_pSkeleton(CTAddRef(pSkeleton))
 , m_animStart(0)
 , m_animEnd(0)
 , m_animTime(0)
{
}

///////////////////////////////////////

cModelAnimationController::~cModelAnimationController()
{
}

///////////////////////////////////////

tResult cModelAnimationController::Advance(double elapsedTime)
{
   if (!m_pAnim)
   {
      return S_FALSE;
   }

   Assert(!(elapsedTime < 0));

   double newAnimTime = m_animTime + elapsedTime;
   if (newAnimTime > m_animEnd)
   {
      double pastEnd = newAnimTime - m_animEnd;
      double animLength = (m_animEnd - m_animStart);
      if (pastEnd > animLength)
      {
         pastEnd = 0;
      }
      newAnimTime = m_animStart + pastEnd;
   }

   size_t nJoints = 0;
   if (AccessSkeleton()->GetJointCount(&nJoints) != S_OK)
   {
      return E_FAIL;
   }

   m_blendMatrices.resize(nJoints);

   for (uint i = 0; i < nJoints; ++i)
   {
      tVec3 position;
      tQuat rotation;
      if (m_pAnim->Interpolate(i, newAnimTime, &position, &rotation) != S_OK)
      {
         return E_FAIL;
      }

      sModelJoint joint;
      if (AccessSkeleton()->GetJoint(i, &joint) != S_OK)
      {
         return E_FAIL;
      }

      tMatrix3 localJointRotMat;
      joint.localRotation.ToMatrix(&localJointRotMat);

      tMatrix3 animRotMat;
      rotation.ToMatrix(&animRotMat);

      tMatrix3 finalRotMat;
      localJointRotMat.Multiply(animRotMat, &finalRotMat);

      tMatrix4 mf;
      mf.m00 = finalRotMat.m00;
      mf.m10 = finalRotMat.m10;
      mf.m20 = finalRotMat.m20;
      mf.m30 = 0;
      mf.m01 = finalRotMat.m01;
      mf.m11 = finalRotMat.m11;
      mf.m21 = finalRotMat.m21;
      mf.m31 = 0;
      mf.m02 = finalRotMat.m02;
      mf.m12 = finalRotMat.m12;
      mf.m22 = finalRotMat.m22;
      mf.m32 = 0;
      mf.m03 = position.x + joint.localTranslation.x;
      mf.m13 = position.y + joint.localTranslation.y;
      mf.m23 = position.z + joint.localTranslation.z;
      mf.m33 = 1;

      int iParent = joint.parentIndex;
      if (iParent < 0)
      {
         m_blendMatrices[i] = mf;
      }
      else
      {
         m_blendMatrices[iParent].Multiply(mf, &m_blendMatrices[i]);
      }
   }

   m_animTime = newAnimTime;

   return S_OK;
}

///////////////////////////////////////

tResult cModelAnimationController::SetAnimation(IModelAnimation * pAnim)
{
   if (pAnim == NULL)
   {
      return E_POINTER;
   }

   pAnim->GetStartEnd(&m_animStart, &m_animEnd);
   m_animTime = m_animStart;
   SafeRelease(m_pAnim);
   m_pAnim = CTAddRef(pAnim);
   return S_OK;
}

///////////////////////////////////////

tResult ModelAnimationControllerCreate(IModelSkeleton * pSkeleton,
                                       IModelAnimationController * * ppAnimController)
{
   if (pSkeleton == NULL || ppAnimController == NULL)
   {
      return E_POINTER;
   }
   size_t nJoints = 0;
   if (pSkeleton->GetJointCount(&nJoints) != S_OK || nJoints == 0)
   {
      return S_FALSE;
   }
   cAutoIPtr<IModelAnimationController> pAnimController = static_cast<IModelAnimationController*>(new cModelAnimationController(pSkeleton));
   if (!pAnimController)
   {
      return E_OUTOFMEMORY;
   }
   return pAnimController.GetPointer(ppAnimController);
}


///////////////////////////////////////////////////////////////////////////////
