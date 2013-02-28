///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FRUSTUM_H
#define INCLUDED_FRUSTUM_H

#include "techdll.h"

#include "plane.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec3;
typedef class cVec3<float> tVec3;

template <typename T> class cMatrix4;
typedef class cMatrix4<float> tMatrix4;

template <typename T> class cAxisAlignedBox;
typedef class cAxisAlignedBox<float> tAxisAlignedBox;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFrustum
//

enum eFrustumPlane
{
   kFP_Left          = 0,
   kFP_Right         = 1,
   kFP_Bottom        = 2,
   kFP_Top           = 3,
   kFP_Near          = 4,
   kFP_Far           = 5,
   kMaxFrustumPlanes = 6,
};

class TECH_API cFrustum
{
public:
   cFrustum();
   cFrustum(const tPlane planes[kMaxFrustumPlanes]);
   ~cFrustum();

   bool PointInFrustum(const tVec3 & point) const;
   bool SphereInFrustum(const tVec3 & center, float radius) const;
   bool BoxInFrustum(const tAxisAlignedBox & box) const;

private:
   tPlane m_planes[kMaxFrustumPlanes];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FRUSTUM_H
