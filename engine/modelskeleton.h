///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELSKELETON_H
#define INCLUDED_MODELSKELETON_H

#include "engine/modelapi.h"
#include "engine/modeltypes.h"

#include <map>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

typedef std::vector<sModelJoint> tModelJoints;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

class cModelSkeleton : public cComObject<IMPLEMENTS(IModelSkeleton)>
{
public:
   cModelSkeleton();
   cModelSkeleton(const cModelSkeleton & other);

   cModelSkeleton(const sModelJoint * pJoints, uint nJoints);

   ~cModelSkeleton();

   const cModelSkeleton & operator =(const cModelSkeleton & other);

   virtual tResult GetJointCount(size_t * pJointCount) const;
   virtual tResult GetJoint(size_t iJoint, sModelJoint * pJoint) const;

   tResult GetBindMatrices(uint nMaxMatrices, tMatrix34 * pMatrices) const;

   tResult AddAnimation(eModelAnimationType type, IModelAnimation * pAnim);
   tResult GetAnimation(eModelAnimationType type, IModelAnimation * * ppAnim) const;

private:
   tModelJoints m_joints;

   typedef std::multimap<eModelAnimationType, IModelAnimation *> tAnimMap;
   tAnimMap m_anims;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELSKELETON_H
