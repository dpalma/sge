///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifndef NDEBUG // entire file

#include "hashtable.h"
#include "hashtabletem.h"

#include "techtime.h"

#include <map>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(HashTableTest);

#define LocalMsg(msg)            DebugMsgEx(HashTableTest,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(HashTableTest,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(HashTableTest,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(HashTableTest,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(HashTableTest,(msg),(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)     DebugMsgIfEx(HashTableTest,(cond),(msg))
#define LocalMsgIf1(cond,msg,a)  DebugMsgIfEx1(HashTableTest,(cond),(msg),(a))
#define LocalMsgIf2(cond,msg,a,b) DebugMsgIfEx2(HashTableTest,(cond),(msg),(a),(b))

///////////////////////////////////////////////////////////////////////////////

static bool coin_flip(void)
{
   return ((Rand() % 2) == 0) ? true : false;
}

static void random_string(char * str, int maxlen)
{
   if (maxlen <= 3)
      return;

   int len = 3 + (Rand() % (maxlen - 3));

   for (int i = 0; i < len; i++)
   {
      if (coin_flip())
         str[i] = 'a' + (Rand() % 26);
      else
         str[i] = 'A' + (Rand() % 26);
   }

   str[len - 1] = 0;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

enum
{
   kNumTests = 50,
   kTestStringLength = 20,
};

///////////////////////////////////////////////////////////////////////////////

class cHashTableTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cHashTableTests);
      CPPUNIT_TEST(TestLookup);
      CPPUNIT_TEST(TestIteration);
      CPPUNIT_TEST(TestCustomKey);
   CPPUNIT_TEST_SUITE_END();

   char testStrings[kNumTests][kTestStringLength];

   cHashTable<const char *, int> m_hashTable;

   void TestLookup();
   void TestIteration();
   void TestCustomKey();

public:
   virtual void setUp();
   virtual void tearDown();
};

////////////////////////////////////////

void cHashTableTests::TestLookup()
{
   int nFailures = 0;

   for (int i = 0; i < kNumTests; i++)
   {
      int index = -1;
      if (!m_hashTable.Lookup(testStrings[i], &index))
      {
         nFailures++;
      }
      else
      {
         CPPUNIT_ASSERT(index == i);
      }
   }

   LocalMsgIf1(nFailures > 0, "%d lookups failed\n", nFailures);
   CPPUNIT_ASSERT(nFailures == 0);
}

////////////////////////////////////////

void cHashTableTests::TestIteration()
{
   const char * key;
   int val;
   int nIterated = 0;
   HANDLE h;
   m_hashTable.IterBegin(&h);
   while (m_hashTable.IterNext(&h, &key, &val))
   {
      CPPUNIT_ASSERT(strcmp(testStrings[val], key) == 0);
      nIterated++;
   }
   m_hashTable.IterEnd(&h);

   LocalMsgIf2(nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CPPUNIT_ASSERT(nIterated == kNumTests);
}

////////////////////////////////////////

class cCustomKey
{
public:
   cCustomKey() : m_a(0), m_b(0), m_c(0) {}
   cCustomKey(uint a, uint b, uint c) : m_a(a), m_b(b), m_c(c) {}
   cCustomKey(const cCustomKey & other) : m_a(other.m_a), m_b(other.m_b), m_c(other.m_c) {}
   const uint * GetAPtr() const { return &m_a; }
   const uint * GetBPtr() const { return &m_b; }
   const uint * GetCPtr() const { return &m_c; }
   bool IsEqual(const cCustomKey & other) const { return (m_a == other.m_a) && (m_b == other.m_b) && (m_c == other.m_c); }
   ulong GetProduct() const { return m_a*m_b*m_c; }
private:
   uint m_a, m_b, m_c;
};

template <>
inline uint cHashFunction<cCustomKey>::Hash(const cCustomKey & k, uint initVal)
{
   uint h = initVal;
   h = hash((byte *)k.GetAPtr(), sizeof(uint), h);
   h = hash((byte *)k.GetBPtr(), sizeof(uint), h);
   h = hash((byte *)k.GetCPtr(), sizeof(uint), h);
   return h;
}

template <>
inline bool cHashFunction<cCustomKey>::Equal(const cCustomKey & a, const cCustomKey & b)
{
   return a.IsEqual(b);
}

void cHashTableTests::TestCustomKey()
{
   cHashTable<cCustomKey, ulong> ckHashTable;

   for (int i = 0; i < 500; i++)
   {
      uint a = rand();
      uint b = rand();
      uint c = rand();
      ulong d = a*b*c;
      CPPUNIT_ASSERT(ckHashTable.Insert(cCustomKey(a,b,c), d));
   }

   cCustomKey key;
   ulong value;

   HANDLE h;
   ckHashTable.IterBegin(&h);
   while (ckHashTable.IterNext(&h, &key, &value))
   {
      CPPUNIT_ASSERT(key.GetProduct() == value);
   }
   ckHashTable.IterEnd(&h);
}

////////////////////////////////////////

void cHashTableTests::setUp()
{
   for (int i = 0; i < kNumTests; i++)
   {
      random_string(testStrings[i], sizeof(testStrings[i]));

      CPPUNIT_ASSERT(m_hashTable.Insert(testStrings[i], i));
   }
}

////////////////////////////////////////

void cHashTableTests::tearDown()
{
   m_hashTable.Clear();
}

///////////////////////////////////////////////////////////////////////////////

class cHashTableSpeedTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cHashTableSpeedTests);
      CPPUNIT_TEST(TestInsertSpeed);
      CPPUNIT_TEST(TestLookupSpeed);
   CPPUNIT_TEST_SUITE_END();

   char m_testStrings[kNumTests][kTestStringLength];

   cHashTable<const char *, int> m_hashTable;
   std::map<const char *, int> m_map;

   struct sTiming
   {
      int64 clockTicks;
      double seconds;
   };

   void RunInsertSpeedTest(sTiming * pHashTableResult, sTiming * pMapResult);
   void TestInsertSpeed();
   void RunLookupSpeedTest(int nLookups, double * pHashTableResult, double * pMapResult);
   void TestLookupSpeed();

public:
   virtual void setUp();
   virtual void tearDown();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cHashTableTests);
CPPUNIT_TEST_SUITE_REGISTRATION(cHashTableSpeedTests);

////////////////////////////////////////

void cHashTableSpeedTests::RunInsertSpeedTest(sTiming * pHashTableResult, sTiming * pMapResult)
{
   CPPUNIT_ASSERT(pHashTableResult != NULL);
   CPPUNIT_ASSERT(pMapResult != NULL);

   int64 startTicks;
   double startSecs;

   m_hashTable.Clear();
   m_map.clear();

   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CPPUNIT_ASSERT(m_hashTable.Insert(m_testStrings[i], i));
      }

      pHashTableResult->clockTicks = ReadTSC() - startTicks;
      pHashTableResult->seconds = TimeGetSecs() - startSecs;
   }

   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CPPUNIT_ASSERT(m_map.insert(std::make_pair((const char *)m_testStrings[i], i)).second);
      }

      pMapResult->clockTicks = ReadTSC() - startTicks;
      pMapResult->seconds = TimeGetSecs() - startSecs;
   }
}

////////////////////////////////////////

void cHashTableSpeedTests::TestInsertSpeed()
{
   const int kNumRuns = 5;
   const double kOneOverNumRuns = 1.0 / kNumRuns;

   sTiming hashTableResult[kNumRuns], mapResult[kNumRuns];
   double hashTableAverageTicks = 0, mapAverageTicks = 0;

   LocalMsg2("cHashTable/std::map Speed Test; inserting %d items; %d runs\n", kNumTests, kNumRuns);
   for (int i = 0; i < kNumRuns; i++)
   {
      RunInsertSpeedTest(&hashTableResult[i], &mapResult[i]);

      LocalMsg3("   [%d] cHashTable: %.5f seconds, %d clock ticks\n",
         i, hashTableResult[i].seconds, hashTableResult[i].clockTicks);

      LocalMsg3("   [%d] std::map:   %.5f seconds, %d  clock ticks\n",
         i, mapResult[i].seconds, mapResult[i].clockTicks);

      hashTableAverageTicks += (double)(long)hashTableResult[i].clockTicks * kOneOverNumRuns;
      mapAverageTicks += (double)(long)mapResult[i].clockTicks * kOneOverNumRuns;
   }

   LocalMsg2("Inserting %d items (average over %d runs):\n", kNumTests, kNumRuns);
   LocalMsg1("   Hash Table: %.2f clock ticks\n", hashTableAverageTicks);
   LocalMsg1("   STL map:    %.2f clock ticks\n", mapAverageTicks);
}

////////////////////////////////////////

void cHashTableSpeedTests::RunLookupSpeedTest(int nLookups, double * pHashTableResult, double * pMapResult)
{
   CPPUNIT_ASSERT(pHashTableResult != NULL);
   CPPUNIT_ASSERT(pMapResult != NULL);

   double oneOverNumLookups = 1.0 / nLookups;

   *pHashTableResult = 0;
   int i;
   for (i = 0; i < nLookups; i++)
   {
      int index = -1;
      int64 start = ReadTSC();
      m_hashTable.Lookup(m_testStrings[i], &index);
      int64 elapsed = ReadTSC() - start;
      *pHashTableResult += (double)(long)elapsed * oneOverNumLookups;
   }

   *pMapResult = 0;
   for (i = 0; i < nLookups; i++)
   {
      int index = -1;
      int64 start = ReadTSC();
      m_map.find(m_testStrings[i]);
      int64 elapsed = ReadTSC() - start;
      *pMapResult += (double)(long)elapsed * oneOverNumLookups;
   }
}

////////////////////////////////////////

void cHashTableSpeedTests::TestLookupSpeed()
{
   double hashTableResult, mapResult;
   RunLookupSpeedTest(kNumTests, &hashTableResult, &mapResult);

   LocalMsg1("Lookup (average over %d lookups):\n", kNumTests);
   LocalMsg1("   Hash Table: %.2f clock ticks\n", hashTableResult);
   LocalMsg1("   STL map:    %.2f clock ticks\n", mapResult);
}

////////////////////////////////////////

void cHashTableSpeedTests::setUp()
{
   for (int i = 0; i < kNumTests; i++)
   {
      random_string(m_testStrings[i], _countof(m_testStrings[i]));
   }
}

////////////////////////////////////////

void cHashTableSpeedTests::tearDown()
{
   m_hashTable.Clear();
}


#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

#endif // !NDEBUG (entire file)
