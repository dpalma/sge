///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "techmath.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#include <ctime>
#endif

#include "dbgalloc.h" // must be last header

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

class cRandTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cRandTests);
      CPPUNIT_TEST(TestFrequency);
   CPPUNIT_TEST_SUITE_END();

   void TestFrequency();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cRandTests);

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
void cRandTests::TestFrequency()
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

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
