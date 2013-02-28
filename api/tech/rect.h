///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RECT_H
#define INCLUDED_RECT_H

#include "point2.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRect
//

template <typename T>
class cRect
{
public:
   typedef T value_type;

   cRect();
   cRect(T l, T t, T r, T b);
   cRect(const cRect & other);
   ~cRect();

   const cRect & operator =(const cRect & other);

   bool operator ==(const cRect & other);

   bool PtInside(T x, T y) const;
   void Offset(T x, T y);

   T GetWidth() const;
   T GetHeight() const;

   template <typename U>
   inline cPoint2<U> GetTopLeft(cPoint2<U> * pVec = NULL) const
   {
      return (pVec != NULL)
         ? (*pVec = cPoint2<U>(static_cast<U>(left), static_cast<U>(top)))
         : cPoint2<U>(static_cast<U>(left), static_cast<U>(top));
   }

   template <typename U>
   inline cPoint2<U> GetTopRight(cPoint2<U> * pVec = NULL) const
   {
      return (pVec != NULL)
         ? (*pVec = cPoint2<U>(static_cast<U>(right), static_cast<U>(top)))
         : cPoint2<U>(static_cast<U>(right), static_cast<U>(top));
   }

   template <typename U>
   inline cPoint2<U> GetBottomLeft(cPoint2<U> * pVec = NULL) const
   {
      return (pVec != NULL)
         ? (*pVec = cPoint2<U>(static_cast<U>(left), static_cast<U>(bottom)))
         : cPoint2<U>(static_cast<U>(left), static_cast<U>(bottom));
   }

   template <typename U>
   inline cPoint2<U> GetBottomRight(cPoint2<U> * pVec = NULL) const
   {
      return (pVec != NULL)
         ? (*pVec = cPoint2<U>(static_cast<U>(right), static_cast<U>(bottom)))
         : cPoint2<U>(static_cast<U>(right), static_cast<U>(bottom));
   }

   union
   {
      struct
      {
         value_type left, top, right, bottom;
      };
      value_type r[4];
   };
};

///////////////////////////////////////

typedef cRect<int> tRecti;
typedef cRect<float> tRectf;

///////////////////////////////////////

template <typename T>
inline cRect<T>::cRect()
{
}

///////////////////////////////////////

template <typename T>
inline cRect<T>::cRect(T l, T t, T r, T b)
 : left(l), top(t), right(r), bottom(b)
{
}

///////////////////////////////////////

template <typename T>
inline cRect<T>::cRect(const cRect & other)
 : left(other.left), top(other.top), right(other.right), bottom(other.bottom)
{
}

///////////////////////////////////////

template <typename T>
inline cRect<T>::~cRect()
{
}
///////////////////////////////////////

template <typename T>
inline const cRect<T> & cRect<T>::operator =(const cRect & other)
{
   left = other.left;
   top = other.top;
   right = other.right;
   bottom = other.bottom;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline bool cRect<T>::operator ==(const cRect & other)
{
   return (left == other.left) && 
      (top == other.top) &&
      (right == other.right) &&
      (bottom == other.bottom);
}

///////////////////////////////////////

template <typename T>
inline bool cRect<T>::PtInside(T x, T y) const
{
   return ((x >= left) && (x <= right) && (y >= top) && (y <= bottom));
}

///////////////////////////////////////

template <typename T>
inline void cRect<T>::Offset(T x, T y)
{
   left += x;
   top += y;
   right += x;
   bottom += y;
}

///////////////////////////////////////

template <typename T>
inline T cRect<T>::GetWidth() const
{
   Assert((right - left) >= 0);
   return right - left;
}

///////////////////////////////////////

template <typename T>
inline T cRect<T>::GetHeight() const
{
   Assert((bottom - top) >= 0);
   return bottom - top;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RECT_H
