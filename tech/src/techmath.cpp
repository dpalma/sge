///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "techmath.h"

#include <cmath>

#ifdef HAVE_CPPUNIT
#include <ctime>
#include <cppunit/extensions/HelperMacros.h>
#endif

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

   uint sqrtn = (uint)sqrt((double)n);

   // test only the odd numbers because we eliminated 2 already
   for (uint i = 3; i <= sqrtn; i += 2)
   {
      if ((n % i) == 0)
         return false; // 'n' is divisible by 'i' so cannot be prime
   }

   return true;
}


///////////////////////////////////////////////////////////////////////////////

uint g_a;

static const uint g_b = 523786821;

void SeedRand(uint seed)
{
   g_a = seed;
}

uint Rand()
{
   return g_a = g_a * g_b + 1;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cMathTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cMathTests);
      CPPUNIT_TEST(TestRandBitFrequency);
      CPPUNIT_TEST(TestAlmostEqual);
   CPPUNIT_TEST_SUITE_END();

   void TestRandBitFrequency();
   void TestAlmostEqual();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cMathTests);

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
void cMathTests::TestRandBitFrequency()
{
   SeedRand(time(NULL));

   const uint kNumTests = 1000;
   uint nTotalBits = sizeof(uint) * kNumTests * 8;
   uint nOneBits = 0;

   for (int i = 0; i < kNumTests; i++)
   {
      nOneBits += CountOneBits(Rand());
   }

   double onesFraction = (double)nOneBits / nTotalBits;

   CPPUNIT_ASSERT(fabs(onesFraction - 0.5) < 0.01);
}

///////////////////////////////////////

void cMathTests::TestAlmostEqual()
{
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
         { 3.1415927, 3.141593, 1, true },
         { 3.1415927, 3.14159, 10, false },
         { 3.1415927, 3.14159, 11, true },
      };

      for (int i = 0; i < _countof(floatTests); i++)
      {
         CPPUNIT_ASSERT(AlmostEqual(floatTests[i].a, floatTests[i].b, floatTests[i].maxULP) == floatTests[i].equal);
      }
   }

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
         CPPUNIT_ASSERT(AlmostEqual(doubleTests[i].a, doubleTests[i].b, doubleTests[i].maxULP) == doubleTests[i].equal);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT
