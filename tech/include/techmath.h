///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHMATH_H
#define INCLUDED_TECHMATH_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

TECH_API bool IsPrime(uint n);

///////////////////////////////////////////////////////////////////////////////

TECH_API void SeedRand(uint seed);

TECH_API uint Rand();

///////////////////////////////////////////////////////////////////////////////

const float kPi = 3.1415926535897932384626433832795f;
const float kRadiansPerDegree = 0.017453292519943295769236907684886f; // pi / 180
#define Rad2Deg(p) ((p)/kRadiansPerDegree)
#define Deg2Rad(p) ((p)*kRadiansPerDegree)

template <typename T>
inline T NearestPowerOfTwo(T t)
{
   T n = (T)(log(t) / log(2));
   return 1 << n;
}

inline bool IsPowerOfTwo(long n)
{
    return (n & -n) == n;
}

inline int Round(float floatval)
{
   int intval;
#if defined(_MSC_VER)
   __asm
   {
      fld   floatval
      fistp intval
   }
#elif defined(__GNUC__)
   asm
   (
      "flds    %1\n"
      "fistpl  %0\n"
      :"=m"(intval)
      :"m"(floatval)
   );
#else
#error ("Need floating point to integer conversion for platform")
#endif
   return intval;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHMATH_H
