///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ANIMATION_H
#define INCLUDED_ANIMATION_H

#include "enginedll.h"
#include "combase.h"

#include "quat.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IKeyFrameAnimation);
F_DECLARE_INTERFACE(IKeyFrameInterpolator);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IKeyFrameAnimation
//

interface IKeyFrameAnimation : IUnknown
{
   virtual tResult GetInterpolator(int index, IKeyFrameInterpolator * * ppInterpolator) const = 0;
};

ENGINE_API tResult KeyFrameAnimationCreate(IKeyFrameInterpolator * * ppInterpolators, uint nInterpolators,
                                           IKeyFrameAnimation * * ppAnimation);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IKeyFrameInterpolator
//

typedef double tTime;

struct sKeyFrame
{
   tTime time;
   tVec3 translation;
   tQuat rotation;
};

interface IKeyFrameInterpolator : IUnknown
{
   virtual tTime GetPeriod() const = 0;

   virtual tResult GetKeyFrames(sKeyFrame * pKeyFrames, uint * pnKeyFrames) const = 0;

   virtual tResult Interpolate(tTime time, sKeyFrame * pInterpFrame) const = 0;
};

///////////////////////////////////////

ENGINE_API tResult KeyFrameInterpolatorCreate(const char * pszName,
                                              const sKeyFrame * pKeys, uint nKeys,
                                              IKeyFrameInterpolator * * ppInterpolator);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ANIMATION_H
