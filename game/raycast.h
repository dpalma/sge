///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RAYCAST_H
#define INCLUDED_RAYCAST_H

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

bool RayIntersectSphere(const tVec3 & rayOrigin, const tVec3 & rayDir,
                        const tVec3 & sphereCenter, float sphereRadius,
                        tVec3 * pIntersect);

bool RayIntersectPlane(const tVec3 & rayOrigin, const tVec3 & rayDir,
                       const tVec3 & planeNormal, tVec3::value_type planeD,
                       tVec3 * pIntersect);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RAYCAST_H
