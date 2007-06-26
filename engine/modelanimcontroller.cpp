///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelanimcontroller.h"

#include "engine/modeltypes.h"

#include "tech/dbgalloc.h" // must be last header


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

tResult cModelAnimationController::Advance(double elapsedTime, uint nMaxMatrices, tMatrix34 * pMatrices)
{
   if (elapsedTime < 0)
   {
      return E_INVALIDARG;
   }

   if (pMatrices == NULL)
   {
      return E_POINTER;
   }

   if (!m_pAnim)
   {
      return E_FAIL;
   }

   size_t nJoints = 0;
   if (AccessSkeleton()->GetJointCount(&nJoints) != S_OK)
   {
      return E_FAIL;
   }

   if (nMaxMatrices < nJoints)
   {
      ErrorMsg2("Need %d blend matrices (given only %d)\n", nJoints, nMaxMatrices);
      return E_INVALIDARG;
   }

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

      tMatrix34 mf;
      mf.SetRotation(finalRotMat);
      mf.SetTranslation(position + joint.localTranslation);

      int iParent = joint.parentIndex;
      if (iParent < 0)
      {
         pMatrices[i] = mf;
      }
      else
      {
         pMatrices[iParent].Compose(mf, &pMatrices[i]);
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
