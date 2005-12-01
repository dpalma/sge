///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelanim.h"
#include "model.h"

#include "matrix4.h"

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
// CLASS: cModelAnimationController
//

///////////////////////////////////////

cModelAnimationController::cModelAnimationController(cModel * pModel)
 : m_pModel(pModel) // TODO: use CTAddRef when IModel becomes a COM interface
 , m_animTime(0)
 , m_animLength(0)
{
   m_animLength = m_pModel->GetTotalAnimationLength();
}

///////////////////////////////////////

cModelAnimationController::~cModelAnimationController()
{
}

///////////////////////////////////////

tResult cModelAnimationController::GetModel(cModel * * ppModel) const
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }
   if (m_pModel == NULL)
   {
      return S_FALSE;
   }
   *ppModel = m_pModel;
   return S_OK;
}

///////////////////////////////////////

tResult cModelAnimationController::SetModel(cModel * pModel)
{
   if (pModel == NULL)
   {
      return E_POINTER;
   }
   m_pModel = pModel; // TODO: use CTAddRef when IModel becomes a COM interface
   m_animTime = 0;
   m_animLength = m_pModel->GetTotalAnimationLength();
   return S_OK;
}

///////////////////////////////////////

tResult cModelAnimationController::Advance(double elapsedTime)
{
   Assert(!(elapsedTime < 0));

   m_animTime += elapsedTime;
   while (m_animTime > m_animLength)
   {
      m_animTime -= m_animLength;
   }

   m_pModel->InterpolateJointMatrices(m_animTime, &m_blendMatrices);
   return S_OK;
}

///////////////////////////////////////

tResult ModelAnimationControllerCreate(cModel * pModel, IModelAnimationController * * ppAnimController)
{
   if (pModel == NULL || ppAnimController == NULL)
   {
      return E_POINTER;
   }
   if (!pModel->IsAnimated())
   {
      return S_FALSE;
   }
   cAutoIPtr<IModelAnimationController> pAnimController;
   pAnimController = static_cast<IModelAnimationController*>(new cModelAnimationController(pModel));
   if (!pAnimController)
   {
      return E_OUTOFMEMORY;
   }
   *ppAnimController = CTAddRef(pAnimController);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
