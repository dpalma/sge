///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ANIMATION_H
#define INCLUDED_ANIMATION_H

#include "techdll.h"
#include "combase.h"

#include "quat.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IKeyFrameInterpolator);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IKeyFrameInterpolator
//

typedef double tTime;

struct sKeyFrameVec3
{
   tTime time;
   tVec3 value;
};

struct sKeyFrameQuat
{
   tTime time;
   tQuat value;
};

interface IKeyFrameInterpolator : IUnknown
{
   virtual tTime GetPeriod() const = 0;

   virtual tResult GetScaleKeys(sKeyFrameVec3 * pScaleKeys, uint * pnScaleKeys) const = 0;
   virtual tResult GetRotationKeys(sKeyFrameQuat * pRotationKeys, uint * pnRotationKeys) const = 0;
   virtual tResult GetTranslationKeys(sKeyFrameVec3 * pTranslationKeys, uint * pnTranslationKeys) const = 0;

   virtual tResult Interpolate(tTime time, tVec3 * pScale, tQuat * pRotation, tVec3 * pTranslation) = 0;
};

///////////////////////////////////////

TECH_API tResult KeyFrameInterpolatorCreate(const char * pszName,
                                            const sKeyFrameVec3 * pScaleKeys, uint nScaleKeys,
                                            const sKeyFrameQuat * pRotationKeys, uint nRotationKeys,
                                            const sKeyFrameVec3 * pTranslationKeys, uint nTranslationKeys,
                                            IKeyFrameInterpolator * * ppInterpolator);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ANIMATION_H
