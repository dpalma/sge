////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RAY_INL
#define INCLUDED_RAY_INL

#include "tech/point3.inl"
#include "tech/techmath.h"

#include <cfloat>

#ifdef _MSC_VER
#pragma once
#endif

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRay
//

////////////////////////////////////////

template <typename T>
cRay<T>::cRay()
{
}

////////////////////////////////////////

template <typename T>
cRay<T>::cRay(const cPoint3<T> & origin, const cVec3<T> & direction)
 : m_origin(origin)
 , m_direction(direction)
{
}

////////////////////////////////////////

template <typename T>
cRay<T>::cRay(const cRay & ray)
 : m_origin(ray.m_origin)
 , m_direction(ray.m_direction)
{
}

////////////////////////////////////////

template <typename T>
const cRay<T> & cRay<T>::operator =(const cRay & ray)
{
   m_origin = ray.m_origin;
   m_direction = ray.m_direction;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cPoint3<T> & cRay<T>::GetOrigin() const
{
   return m_origin;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cRay<T>::GetDirection() const
{
   return m_direction;
}

////////////////////////////////////////

template <typename T>
bool cRay<T>::IntersectsSphere(const cPoint3<T> & center, 
                               typename cVec3<T>::value_type radius, 
                               cPoint3<T> * pIntersection /*=NULL*/) const
{
   // Don't need to calculate the 'a' term because the ray is a unit vector

   // Calculate the 'b' term 
   float b = 2 *((GetDirection().x * (GetOrigin().x - center.x)) +
                 (GetDirection().y * (GetOrigin().y - center.y)) +
                 (GetDirection().z * (GetOrigin().z - center.z)));

   // Calculate the 'c' term 
   float c = sqr(GetOrigin().x - center.x) +
             sqr(GetOrigin().y - center.y) +
             sqr(GetOrigin().z - center.z) -
             sqr(radius);

   // Calculate the discriminant. If discriminant not positive
   // then the ray does not hit the sphere. Return a t of 0.
   float discr = (sqr(b) - 4*c);
   if (discr <= FLT_EPSILON)
      return false;

   // Solve the quadratic equation. You remember the roots are:
   // (-b +/- sqrt(b^2 - 4*a*c)) / 2a.
   discr = sqrtf(discr);
   float t0 = (-b - discr) * 0.5f;

   // If t0 is positive we are done. If not, we must calculate
   // the other root t1.
   if (t0 > -FLT_EPSILON)
   {
      if (pIntersection != NULL)
      {
         *pIntersection = cPoint3<float>(GetOrigin() + (GetDirection() * t0));
      }
      return true;
   }

   float t1 = (-b + discr) * 0.5f;
   if (t1 > -FLT_EPSILON)
   {
      if (pIntersection != NULL)
      {
         *pIntersection = cPoint3<float>(GetOrigin() + (GetDirection() * t1));
      }
      return true;
   }

   return false;
}

////////////////////////////////////////

template <typename T>
bool cRay<T>::IntersectsPlane(const cVec3<T> & normal, 
                              typename cVec3<T>::value_type d,
                              cPoint3<T> * pIntersection /*=NULL*/) const
{
   cVec3<T>::value_type dotProd = normal.Dot(GetDirection());

   if (dotProd == 0)
      return false;

   cVec3<T>::value_type t = -(normal.x * GetOrigin().x +
                              normal.y * GetOrigin().y +
                              normal.z * GetOrigin().z + d) / dotProd;

   if (t < 0)
   {
      return false;
   }

   if (pIntersection != NULL)
   {
      *pIntersection = GetOrigin() + (GetDirection() * t);
   }

   return true;
}

////////////////////////////////////////
// "An Introduction To Ray Tracing", Ed. by Andrew Glassner, 1989.

#define Max3(a,b,c) Max(Max(a,b),c)

template <typename T>
bool cRay<T>::IntersectsTriangle(const cPoint3<T> & v1,
                                 const cPoint3<T> & v2,
                                 const cPoint3<T> & v3,
                                 cPoint3<T> * pIntersection /*=NULL*/) const
{
   cVec3<T> a(v2 - v1), b(v3 - v1);
   cVec3<T> n(a.Cross(b));
   n.Normalize();

   float d = n.Dot(cVec3<T>(-v1.x,-v1.y,-v1.z));

   cPoint3<T> pi;
   if (!IntersectsPlane(n, d, &pi))
   {
      return false;
   }

   int coordIndices[2];

   float anx = Abs(n.x);
   float any = Abs(n.y);
   float anz = Abs(n.z);

   float dc = Max3(anx, any, anz);
   if (dc == anx)
   {
      coordIndices[0] = 1;
      coordIndices[1] = 2;
   }
   else if (dc == any)
   {
      coordIndices[0] = 0;
      coordIndices[1] = 2;
   }
   else
   {
      coordIndices[0] = 0;
      coordIndices[1] = 1;
   }

   float v1px = v1.xyz[coordIndices[0]] - pi.xyz[coordIndices[0]],
         v1py = v1.xyz[coordIndices[1]] - pi.xyz[coordIndices[1]];
   float v2px = v2.xyz[coordIndices[0]] - pi.xyz[coordIndices[0]],
         v2py = v2.xyz[coordIndices[1]] - pi.xyz[coordIndices[1]];
   float v3px = v3.xyz[coordIndices[0]] - pi.xyz[coordIndices[0]],
         v3py = v3.xyz[coordIndices[1]] - pi.xyz[coordIndices[1]];

   int nc = 0;
   int sh = v1py < 0 ? -1 : 1, nsh;

   struct { float ua, va, ub, vb; } edges[3] =
   {
      { v1px, v1py, v2px, v2py },
      { v2px, v2py, v3px, v3py },
      { v3px, v3py, v1px, v1py }
   };

   for (int i = 0; i < _countof(edges); i++)
   {
      nsh = edges[i].vb < 0 ? -1 : 1;
      if (sh != nsh)
      {
         if (edges[i].ua > 0 && edges[i].ub > 0)
         {
            nc++;
         }
         else if (edges[i].ua > 0 || edges[i].ub > 0)
         {
            float xi = edges[i].ua - edges[i].va * (edges[i].ub - edges[i].ua) / (edges[i].vb - edges[i].va);
            if (xi > 0)
            {
               nc++;
            }
         }
         sh = nsh;
      }
   }

   if (nc & 1)
   {
      if (pIntersection != NULL)
      {
         *pIntersection = pi;
      }
      return true;
   }

   return false;
}

////////////////////////////////////////
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
//
// The Ray is defined as before in terms of Ro, Rd Then the algorithm is as follows:
//
// set Tnear = - infinity, Tfar = infinity
//
// For each pair of planes P associated with X, Y, and Z do:
// (example using X planes)
// if direction Xd = 0 then the ray is parallel to the X planes, so
// if origin Xo is not between the slabs ( Xo < Xl or Xo > Xh) then return false
// else, if the ray is not parallel to the plane then
// begin
// compute the intersection distance of the planes
// T1 = (Xl - Xo) / Xd
// T2 = (Xh - Xo) / Xd
// If T1 > T2 swap (T1, T2) /* since T1 intersection with near plane */
// If T1 > Tnear set Tnear =T1 /* want largest Tnear */
// If T2 < Tfar set Tfar="T2" /* want smallest Tfar */
// If Tnear > Tfar box is missed so return false
// If Tfar < 0 box is behind ray return false end
//
// end of for loop If Box survived all above tests, return true with intersection point Tnear and exit point Tfar.

template <typename T>
bool cRay<T>::IntersectsAxisAlignedBox(const cAxisAlignedBox<T> & box,
                                       T * pTNear /*=NULL*/,
                                       T * pTFar /*=NULL*/) const
{
   float tNear = -FLT_MAX, tFar = FLT_MAX;

   // For x, y, z
   for (int i = 0; i < 3; i++)
   {
      float d = m_direction.v[i]; // direction component
      float o = m_origin.xyz[i]; // origin component
      float mn = box.GetMins().xyz[i]; // box minimum component
      float mx = box.GetMaxs().xyz[i]; // box maximum component

      if (AlmostEqual(d, 0))
      {
         if ((d < mn) || (d > mx))
         {
            return false;
         }
      }

      float t1 = (mn - o) / d;
      float t2 = (mx - o) / d;
      if (t1 > t2)
      {
         float temp = t1;
         t1 = t2;
         t2 = temp;
      }
      if (t1 > tNear)
      {
         tNear = t1;
      }
      if (t2 < tFar)
      {
         tFar = t2;
      }
      if ((tNear > tFar) || (tFar < 0))
      {
         return false;
      }
   }

   if (pTNear != NULL)
   {
      *pTNear = tNear;
   }
   if (pTFar != NULL)
   {
      *pTFar = tFar;
   }
   return true;
}

#include "undbgalloc.h"

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RAY_INL
