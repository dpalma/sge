///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "techmath.h"
#include <cmath>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool IsPrime(uint n)
{
   if (n <= 1) // 0, 1 are NOT primes
      return false;

   if (n <= 3) // 2, 3 are primes
      return true;

   if ((n % 2) == 0) // anything evenly divisible by 2 is NOT a prime
      return false;

   uint sqrtn = (uint)sqrt(n);

   // test only the odd numbers because we eliminated 2 already
   for (uint i = 3; i <= sqrtn; i += 2)
   {
      if ((n % i) == 0)
         return false; // 'n' is divisible by 'i' so cannot be prime
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////
