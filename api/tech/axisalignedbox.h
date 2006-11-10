///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AXISALIGNEDBOX_H
#define INCLUDED_AXISALIGNEDBOX_H

#include "techdll.h"

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cAxisAlignedBox
//

template <typename T>
class cAxisAlignedBox
{
public:
   cAxisAlignedBox();
   cAxisAlignedBox(const cVec3<T> & mins, const cVec3<T> & maxs);
   cAxisAlignedBox(const cAxisAlignedBox & other);
   ~cAxisAlignedBox();

   const cAxisAlignedBox & operator =(const cAxisAlignedBox & other);

   const cVec3<T> & GetMins() const;
   const cVec3<T> & GetMaxs() const;

   void GetCentroid(cVec3<T> * pCentroid) const;

   T GetVolume() const;

   T GetBiggestDimension() const;

   bool IsEmpty() const; // Zero length in at least one dimension? (In other words, volume is zero)

   bool Intersects(const cAxisAlignedBox & other, cAxisAlignedBox * pIntersection = NULL) const;

   cAxisAlignedBox Union(const cAxisAlignedBox & other) const;

   void Offset(const cVec3<T> & offset);

private:
   cVec3<T> m_mins, m_maxs;
};

////////////////////////////////////////

template <typename T>
cAxisAlignedBox<T>::cAxisAlignedBox()
{
}

////////////////////////////////////////

template <typename T>
cAxisAlignedBox<T>::cAxisAlignedBox(const cVec3<T> & mins, const cVec3<T> & maxs)
 : m_mins(mins)
 , m_maxs(maxs)
{
}

////////////////////////////////////////

template <typename T>
cAxisAlignedBox<T>::cAxisAlignedBox(const cAxisAlignedBox & other)
 : m_mins(other.m_mins)
 , m_maxs(other.m_maxs)
{
}

////////////////////////////////////////

template <typename T>
cAxisAlignedBox<T>::~cAxisAlignedBox()
{
}

////////////////////////////////////////

template <typename T>
const cAxisAlignedBox<T> & cAxisAlignedBox<T>::operator =(const cAxisAlignedBox & other)
{
   m_mins = other.m_mins;
   m_maxs = other.m_maxs;
   return *this;
}

////////////////////////////////////////

template <typename T>
inline const cVec3<T> & cAxisAlignedBox<T>::GetMins() const
{
   return m_mins;
}

////////////////////////////////////////

template <typename T>
inline const cVec3<T> & cAxisAlignedBox<T>::GetMaxs() const
{
   return m_maxs;
}

////////////////////////////////////////

template <typename T>
void cAxisAlignedBox<T>::GetCentroid(cVec3<T> * pCentroid) const
{
   if (pCentroid != NULL)
   {
      *pCentroid = tVec3((m_mins.x + m_maxs.x) / 2, (m_mins.y + m_maxs.y) / 2, (m_mins.z + m_maxs.z) / 2);
   }
}

////////////////////////////////////////

template <typename T>
T cAxisAlignedBox<T>::GetVolume() const
{
   return (m_maxs.x - m_mins.x)
      * (m_maxs.y - m_mins.y)
      * (m_maxs.z - m_mins.z);
}

////////////////////////////////////////

template <typename T>
T cAxisAlignedBox<T>::GetBiggestDimension() const
{
   float dimX = (m_maxs.x - m_mins.x), dimY = (m_maxs.y - m_mins.y), dimZ = (m_maxs.z - m_mins.z);
   return Max(dimX, Max(dimY, dimZ));
}

////////////////////////////////////////

template <typename T>
bool cAxisAlignedBox<T>::IsEmpty() const
{
   return AlmostEqual(m_mins.x, m_maxs.x)
      || AlmostEqual(m_mins.y, m_maxs.y)
      || AlmostEqual(m_mins.z, m_maxs.z);
}

////////////////////////////////////////

template <typename T>
bool cAxisAlignedBox<T>::Intersects(const cAxisAlignedBox & other, cAxisAlignedBox * pIntersection) const
{
   cAxisAlignedBox<T> intersection(
      cVec3<T>(Max(m_mins.x, other.m_mins.x), Max(m_mins.y, other.m_mins.y), Max(m_mins.z, other.m_mins.z)),
      cVec3<T>(Min(m_maxs.x, other.m_maxs.x), Min(m_maxs.y, other.m_maxs.y), Min(m_maxs.z, other.m_maxs.z)));
   if (intersection.GetMins().x < intersection.GetMaxs().x
      || intersection.GetMins().y < intersection.GetMaxs().y
      || intersection.GetMins().z < intersection.GetMaxs().z)
   {
      return false;
   }
   if (pIntersection != NULL)
   {
      *pIntersection = intersection;
   }
   return true;
}

////////////////////////////////////////

template <typename T>
cAxisAlignedBox<T> cAxisAlignedBox<T>::Union(const cAxisAlignedBox & other) const
{
   return cAxisAlignedBox<T>(
      cVec3<T>(Min(m_mins.x, other.m_mins.x), Min(m_mins.y, other.m_mins.y), Min(m_mins.z, other.m_mins.z)),
      cVec3<T>(Max(m_maxs.x, other.m_maxs.x), Max(m_maxs.y, other.m_maxs.y), Max(m_maxs.z, other.m_maxs.z)));
}

////////////////////////////////////////

template <typename T>
void cAxisAlignedBox<T>::Offset(const cVec3<T> & offset)
{
   m_mins += offset;
   m_maxs += offset;
}

////////////////////////////////////////

typedef cAxisAlignedBox<float> tAxisAlignedBox;

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AXISALIGNEDBOX_H
