///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RECT_H
#define INCLUDED_RECT_H

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
   cRect();
   cRect(T l, T t, T r, T b);
   cRect(const cRect & other);
   const cRect & operator =(const cRect & other);
   bool operator ==(const cRect & other);
   bool PtInside(T x, T y) const;
   void Offset(T x, T y);
   T GetWidth() const;
   T GetHeight() const;
   T left, top, right, bottom;
};

///////////////////////////////////////

typedef cRect<int> tRect;
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
