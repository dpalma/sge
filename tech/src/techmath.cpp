///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/techmath.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cmath>
#include <ctime>

#include "tech/dbgalloc.h" // must be last header

// REFERENCES
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html

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
// A Mersenne-Twister pseudo-random number generator

////////////////////////////////////////

#define M 397
#define MATRIX_A 0x9908b0dfUL   // constant vector a
#define UPPER_MASK 0x80000000UL // most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // least significant r bits

////////////////////////////////////////

cRand::cRand()
 : m_mti(N + 1)
{
}

////////////////////////////////////////

cRand::cRand(ulong seed)
 : m_mti(N + 1)
{
   Seed(seed);
}

////////////////////////////////////////

void cRand::Seed(ulong seed)
{
   m_mt[0]= seed & 0xffffffffUL;
   for (m_mti = 1; m_mti < N; m_mti++)
   {
      m_mt[m_mti] = (1812433253UL * (m_mt[m_mti-1] ^ (m_mt[m_mti-1] >> 30)) + m_mti); 
      // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
      // In the previous versions, MSBs of the seed affect
      // only MSBs of the array mt[].
      m_mt[m_mti] &= 0xffffffffUL;
      // for >32 bit machines
   }
}

////////////////////////////////////////

void cRand::Seed(ulong keys[], int nKeys)
{
   Seed(19650218UL);
   int i = 1, j = 0, k = (N > nKeys ? N : nKeys);
   for (; k; k--)
   {
      m_mt[i] = (m_mt[i] ^ ((m_mt[i-1] ^ (m_mt[i-1] >> 30)) * 1664525UL)) + keys[j] + j; // non linear
      m_mt[i] &= 0xffffffffUL; // for WORDSIZE > 32 machines
      i++;
      j++;
      if (i >= N)
      {
         m_mt[0] = m_mt[N-1];
         i=1;
      }
      if (j >= nKeys)
      {
         j=0;
      }
   }
   for (k = N-1; k; k--)
   {
      m_mt[i] = (m_mt[i] ^ ((m_mt[i-1] ^ (m_mt[i-1] >> 30)) * 1566083941UL)) - i; // non linear
      m_mt[i] &= 0xffffffffUL; // for WORDSIZE > 32 machines
      i++;
      if (i >= N)
      {
         m_mt[0] = m_mt[N-1];
         i = 1;
      }
   }

   m_mt[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
}

////////////////////////////////////////

ulong cRand::Next()
{
   ulong y;
   static ulong mag01[2] = { 0x0UL, MATRIX_A };
   // mag01[x] = x * MATRIX_A  for x=0,1

   // generate N words at one time
   if (m_mti >= N)
   {
      // if Seed() has not been called, use a default initial seed
      if (m_mti == N+1)
      {
         Seed(5489UL);
      }

      int kk;
      for (kk = 0; kk < N-M; kk++)
      {
         y = (m_mt[kk]&UPPER_MASK)|(m_mt[kk+1]&LOWER_MASK);
         m_mt[kk] = m_mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      for (; kk < N-1; kk++)
      {
         y = (m_mt[kk]&UPPER_MASK)|(m_mt[kk+1]&LOWER_MASK);
         m_mt[kk] = m_mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      y = (m_mt[N-1]&UPPER_MASK)|(m_mt[0]&LOWER_MASK);
      m_mt[N-1] = m_mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

      m_mti = 0;
   }

   y = m_mt[m_mti++];

   // Tempering
   y ^= (y >> 11);
   y ^= (y << 7) & 0x9d2c5680UL;
   y ^= (y << 15) & 0xefc60000UL;
   y ^= (y >> 18);

   return y;
}

////////////////////////////////////////

float cRand::NextFloat()
{
   static const float kOneOverMaxLong = 1.0f / 4294967295.0f;
   return Next() * kOneOverMaxLong; 
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

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
   cRand r(static_cast<ulong>(time(NULL)));

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

#endif // HAVE_UNITTESTPP
