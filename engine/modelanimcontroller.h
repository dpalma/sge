///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELANIMCONTROLLER_H
#define INCLUDED_MODELANIMCONTROLLER_H

#include "engine/modelapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelAnimationController
//

class cModelAnimationController : public cComObject<IMPLEMENTS(IModelAnimationController)>
{
   cModelAnimationController(const cModelAnimationController & other);
   const cModelAnimationController & operator =(const cModelAnimationController & other);

public:
   cModelAnimationController(IModelSkeleton * pSkeleton);
   ~cModelAnimationController();

   virtual tResult Advance(double elapsedTime, uint nMaxMatrices, tMatrix34 * pMatrices);

   virtual tResult SetAnimation(IModelAnimation * pAnim);

private:
   IModelSkeleton * AccessSkeleton() { return m_pSkeleton; }
   const IModelSkeleton * AccessSkeleton() const { return m_pSkeleton; }

   cAutoIPtr<IModelSkeleton> m_pSkeleton;
   cAutoIPtr<IModelAnimation> m_pAnim;
   double m_animStart, m_animEnd, m_animTime;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELANIMCONTROLLER_H
