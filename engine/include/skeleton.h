///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SKELETON_H
#define INCLUDED_SKELETON_H

#include "enginedll.h"
#include "comtools.h"
#include "matrix4.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IKeyFrameAnimation);

F_DECLARE_INTERFACE(ISkeleton);

typedef std::vector<tMatrix4> tMatrices;

const int kMaxBoneName = 32;

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISkeleton
//

interface ISkeleton : IUnknown
{
   virtual int GetBoneCount() const = 0;
   virtual const char * GetBoneName(int index) const = 0;
   virtual const tMatrix4 & GetBoneWorldTransform(int index) const = 0;

   virtual tResult SetAnimation(IKeyFrameAnimation * pAnimation) = 0;
   virtual tResult GetAnimation(IKeyFrameAnimation * * ppAnimation) = 0;

   // TODO: Pull out animation-related methods into separate interfaces
   virtual void GetBoneMatrices(float percent, tMatrices * pBoneMatrices) const = 0;
};

////////////////////////////////////////////////////////////////////////////////

struct sBoneInfo
{
   char name[kMaxBoneName];
   int parentIndex; // TODO: Better to specify parent by index or string name?
   tMatrix4 localTransform;
};

///////////////////////////////////////

ENGINE_API tResult SkeletonCreate(const sBoneInfo * pBones, uint nBones, 
   ISkeleton * * ppSkeleton);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SKELETON_H
