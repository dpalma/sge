///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELSKELETON_H
#define INCLUDED_MODELSKELETON_H

#include "modelapi.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

class cModelSkeleton : public cComObject<IMPLEMENTS(IModelSkeleton)>
{
public:
   cModelSkeleton();
   cModelSkeleton(const cModelSkeleton & other);

   cModelSkeleton(const tModelJoints & joints);

   ~cModelSkeleton();

   const cModelSkeleton & operator =(const cModelSkeleton & other);

   virtual tResult GetJointCount(size_t * pJointCount) const;
   virtual tResult GetJoint(size_t iJoint, sModelJoint * pJoint) const;

   tResult GetBindMatrices(size_t nMaxMatrices, tMatrix4 * pMatrices) const;

   tResult AddAnimation(eModelAnimationType type, IModelAnimation * pAnim);
   tResult GetAnimation(eModelAnimationType type, IModelAnimation * * ppAnim) const;

private:
   tModelJoints m_joints;

   typedef std::multimap<eModelAnimationType, IModelAnimation *> tAnimMap;
   tAnimMap m_anims;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELSKELETON_H
