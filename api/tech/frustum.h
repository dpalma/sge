///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FRUSTUM_H
#define INCLUDED_FRUSTUM_H

#include "techdll.h"

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
// CLASS: cPlane
//

class TECH_API cPlane
{
public:
   void Normalize();

   float a, b, c, d;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFrustum
//

class TECH_API cFrustum
{
public:
   void ExtractPlanes(const tMatrix4 & viewProjection);

   bool PointInFrustum(const tVec3 & point) const;
   bool SphereInFrustum(const tVec3 & center, float radius) const;
   bool BoxInFrustum(const tAxisAlignedBox & box) const;

private:
   cPlane m_planes[6];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FRUSTUM_H
