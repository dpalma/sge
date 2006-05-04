///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "techmath.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cmath>
#include <ctime>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

bool IsPrime(uint n)
{
   if (n <= 1) // 0, 1 are NOT primes
   {
      return false;
   }

   if (n <= 3) // 2, 3 are primes
   {
      return true;
   }

   if ((n & 1) == 0) // anything evenly divisible by 2 is NOT a prime
   {
      return false;
   }

   uint sqrtn = (uint)sqrt((double)n);

   // test only the odd numbers
   for (uint i = 5; i <= sqrtn; i += 2)
   {
      if ((n % i) == 0)
      {
         return false; // 'n' is divisible by 'i' so cannot be prime
      }
   }

   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRand
//

cRand::cRand()
{
}

cRand::cRand(uint seed)
 : m_a(seed)
{
}

void cRand::Seed(uint seed)
{
   m_a = seed;
}

uint cRand::Next()
{
   m_a = m_a * b + 1;
   return m_a;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

TEST(IsPrime)
{
   CHECK(!IsPrime(0));
   CHECK(!IsPrime(1));
   CHECK(IsPrime(2));
   CHECK(IsPrime(3));
   CHECK(!IsPrime(4));
   CHECK(IsPrime(5));
   CHECK(!IsPrime(6));
   CHECK(IsPrime(7));
   CHECK(!IsPrime(8));
}

///////////////////////////////////////

static uint CountOneBits(uint n)
{
   uint count = 0;
   for (uint i = 0; i < (sizeof(uint) * 8); i++)
   {
      if ((1 << i) & n)
      {
         count++;
      }
   }
   return count;
}

// Do a simple "frequency" test on the random number generator.
// See if the bit stream composed of a bunch of random numbers is
// within a tolerance of 50% ones.
TEST(RandBitFrequency)
{
   cRand r(time(NULL));

   const uint kNumTests = 1000;
   uint nTotalBits = sizeof(uint) * kNumTests * 8;
   uint nOneBits = 0;

   for (int i = 0; i < kNumTests; i++)
   {
      nOneBits += CountOneBits(r.Next());
   }

   double onesFraction = (double)nOneBits / nTotalBits;

   CHECK(fabs(onesFraction - 0.5) < 0.01);
}

///////////////////////////////////////

TEST(AlmostEqual)
{
   // Floats
   {
      static const struct
      {
         float a, b;
         int maxULP;
         bool equal;
      }
      floatTests[] =
      {
         { .0f, .0f, 1, true },
         { 3.1415927f, 3.141593f, 1, true },
         { 3.1415927f, 3.14159f, 10, false },
         { 3.1415927f, 3.14159f, 11, true },
      };

      for (int i = 0; i < _countof(floatTests); i++)
      {
         CHECK(AlmostEqual(floatTests[i].a, floatTests[i].b, floatTests[i].maxULP) == floatTests[i].equal);
      }
   }

   // Doubles
   {
      static const struct
      {
         double a, b;
         int maxULP;
         bool equal;
      }
      doubleTests[] =
      {
         { .0f, .0f, 1, true },
         { 3.1415926535897932384626433832795, 3.1415926535897932384626433832795, 1, true },
      };

      for (int i = 0; i < _countof(doubleTests); i++)
      {
         CHECK(AlmostEqual(doubleTests[i].a, doubleTests[i].b, doubleTests[i].maxULP) == doubleTests[i].equal);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNITLITE2
