///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelanim.h"
#include "model.h"

#include "tech/matrix3.h"
#include "tech/matrix4.h"
#include "tech/quat.inl"

#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include <algorithm>
#include <cfloat>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;

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
            *pRot = Slerp(prev->rotation, iter->rotation, static_cast<float>(u));
            *pTrans = Lerp(prev->translation, iter->translation, (tVec3::value_type)u);
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
   std::for_each(m_interpolators.begin(), m_interpolators.end(), mem_fn(&IUnknown::Release));
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
