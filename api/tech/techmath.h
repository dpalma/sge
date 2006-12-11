///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHMATH_H
#define INCLUDED_TECHMATH_H

#include "techdll.h"

#include <cmath>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

TECH_API bool IsPrime(uint n);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRand
//

class TECH_API cRand
{
public:
   cRand();
   cRand(ulong seed);

   void Seed(ulong seed);
   void Seed(ulong keys[], int nKeys);

   ulong Next();
   float NextFloat();

private:
   enum { N = 624 };
   ulong m_mt[N];
   int m_mti;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32_WCE
#define sqrtf(x)  ((float)sqrt((double)(x)))
#define sinf(x)   ((float)sin((double)(x)))
#define cosf(x)   ((float)cos((double)(x)))
#define tanf(x)   ((float)tan((double)(x)))
#define asinf(x)  ((float)asin((double)(x)))
#define acosf(x)  ((float)acos((double)(x)))
#define atanf(x)  ((float)atan((double)(x)))
#endif

#ifndef sqr
#define sqr(x) ((x)*(x))
#endif

const float kPi = 3.1415926535897932384626433832795f;
const float kRadiansPerDegree = 0.017453292519943295769236907684886f; // pi / 180
#define Rad2Deg(p) ((p)/kRadiansPerDegree)
#define Deg2Rad(p) ((p)*kRadiansPerDegree)

template <typename T>
inline T NearestPowerOfTwo(T t)
{
   T n = (T)(log((double)t) / log(2.0));
   return 1 << n;
}

inline bool IsPowerOfTwo(long n)
{
    return (n & -n) == n;
}

inline int FloatToInt(float f)
{
   int i;
#if defined(_MANAGED)
   i = (int)f; // TODO: better way for managed code w/o generating a compile warning?
#elif defined(_MSC_VER)
   __asm
   {
      fld   f
      fistp i
   }
#elif defined(__GNUC__)
   asm
   (
      "flds    %1\n"
      "fistpl  %0\n"
      :"=m"(i)
      :"m"(f)
   );
#else
#error ("Need floating point to integer conversion for platform")
#endif
   return i;
}

inline int DoubleToInt(double d)
{
   return FloatToInt(static_cast<float>(d));
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T Abs(T t)
{
   return (t < 0) ? -t : t;
}

///////////////////////////////////////////////////////////////////////////////
// See http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

const int kFloatMaxUnitsLastPlace = 5;
const int kDoubleMaxUnitsLastPlace = 10;

inline bool AlmostEqual(float a, float b, int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   if (a == b)
   {
      return true; 
   }
   int intDiff = Abs(*(int*)&a - *(int*)&b);
   return (intDiff <= maxUnitsLastPlace);
}

///////////////////////////////////////////////////////////////////////////////

inline bool AlmostEqual(double a, double b, int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   if (a == b)
   {
      return true; 
   }
   int intDiff = Abs(*(int*)&a - *(int*)&b);
   return (intDiff <= maxUnitsLastPlace);
}


///////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
T Lerp(const T & a, const T & b, U t)
{
   return a + (t * (b - a));
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cTimedLerp
//

template <typename T>
class cTimedLerp
{
public:
   cTimedLerp();
   cTimedLerp(const T & start, const T & end, const T & rate);

   void Restart(const T & start, const T & end, const T & rate);

   T Update(T time);

private:
   T m_start, m_end, m_t, m_oneOverTimeSpan;
};

////////////////////////////////////////

template <typename T>
cTimedLerp<T>::cTimedLerp()
{
}

////////////////////////////////////////

template <typename T>
cTimedLerp<T>::cTimedLerp(const T & start, const T & end, const T & rate)
 : m_start(start)
 , m_end(end)
 , m_t(0)
 , m_oneOverTimeSpan((start != end) ? fabs(rate / (end - start)) : 1)
{
}

////////////////////////////////////////

template <typename T>
void cTimedLerp<T>::Restart(const T & start, const T & end, const T & rate)
{
   m_start = start;
   m_end = end;
   m_t = 0;
   if (start != end)
   {
      m_oneOverTimeSpan = fabs(rate / (end - start));
   }
   else
   {
      m_oneOverTimeSpan = 1;
   }
}

////////////////////////////////////////

template <typename T>
T cTimedLerp<T>::Update(T time)
{
   if (m_start == m_end)
   {
      return m_end;
   }
   m_t += (time * m_oneOverTimeSpan);
   if (m_t >= 1)
   {
      m_t = 1;
      return m_end;
   }
   return Lerp<T>(m_start, m_end, m_t);
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHMATH_H
