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

   str[i] = 0;
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
   CPPUNIT_TEST_SUITE_END();

   char testStrings[kNumTests][kTestStringLength];

   cHashTable<const char *, int> m_hashTable;

   void TestLookup();
   void TestIteration();

public:
   virtual void setUp();
   virtual void tearDown();
};

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

   DebugMsgIfEx1(HashTableTest, nFailures > 0, "%d lookups failed\n", nFailures);
   CPPUNIT_ASSERT(nFailures == 0);
}

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

   DebugMsgIfEx2(HashTableTest, nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CPPUNIT_ASSERT(nIterated == kNumTests);
}

void cHashTableTests::setUp()
{
   for (int i = 0; i < kNumTests; i++)
   {
      random_string(testStrings[i], sizeof(testStrings[i]));

      CPPUNIT_ASSERT(m_hashTable.Insert(testStrings[i], i));
   }
}

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

   void RunInsertSpeedTest(int64 * pHashTableResult, int64 * pMapResult);
   void TestInsertSpeed();
   void RunLookupSpeedTest(int nLookups, double * pHashTableResult, double * pMapResult);
   void TestLookupSpeed();

public:
   virtual void setUp();
   virtual void tearDown();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cHashTableTests);
CPPUNIT_TEST_SUITE_REGISTRATION(cHashTableSpeedTests);

void cHashTableSpeedTests::RunInsertSpeedTest(int64 * pHashTableResult, int64 * pMapResult)
{
   CPPUNIT_ASSERT(pHashTableResult != NULL);
   CPPUNIT_ASSERT(pMapResult != NULL);

   int64 start;

   m_hashTable.Clear();
   m_map.clear();

   {
      start = ReadTSC();

      for (int i = 0; i < kNumTests; i++)
      {
         CPPUNIT_ASSERT(m_hashTable.Insert(m_testStrings[i], i));
      }

      *pHashTableResult = ReadTSC() - start;
   }

   {
      start = ReadTSC();

      for (int i = 0; i < kNumTests; i++)
      {
         CPPUNIT_ASSERT(m_map.insert(std::make_pair((const char *)m_testStrings[i], i)).second);
      }

      *pMapResult = ReadTSC() - start;
   }
}

void cHashTableSpeedTests::TestInsertSpeed()
{
   const int kNumRuns = 5;
   const double kOneOverNumRuns = 1.0 / kNumRuns;

   int64 hashTableResult[kNumRuns], mapResult[kNumRuns];
   double hashTableAverage = 0, mapAverage = 0;

   for (int i = 0; i < kNumRuns; i++)
   {
      RunInsertSpeedTest(&hashTableResult[i], &mapResult[i]);
      hashTableAverage += (double)(long)hashTableResult[i] * kOneOverNumRuns;
      mapAverage += (double)(long)mapResult[i] * kOneOverNumRuns;
   }

   DebugMsgEx2(HashTableTest, "Inserting %d items (average over %d runs):\n", kNumTests, kNumRuns);
   DebugMsgEx1(HashTableTest, "   Hash Table: %.2f clock ticks\n", hashTableAverage);
   DebugMsgEx1(HashTableTest, "   STL map:    %.2f clock ticks\n", mapAverage);
}

void cHashTableSpeedTests::RunLookupSpeedTest(int nLookups, double * pHashTableResult, double * pMapResult)
{
   CPPUNIT_ASSERT(pHashTableResult != NULL);
   CPPUNIT_ASSERT(pMapResult != NULL);

   double oneOverNumLookups = 1.0 / nLookups;

   *pHashTableResult = 0;
   for (int i = 0; i < nLookups; i++)
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

void cHashTableSpeedTests::TestLookupSpeed()
{
   double hashTableResult, mapResult;
   RunLookupSpeedTest(kNumTests, &hashTableResult, &mapResult);

   DebugMsgEx1(HashTableTest, "Lookup (average over %d lookups):\n", kNumTests);
   DebugMsgEx1(HashTableTest, "   Hash Table: %.2f clock ticks\n", hashTableResult);
   DebugMsgEx1(HashTableTest, "   STL map:    %.2f clock ticks\n", mapResult);
}

void cHashTableSpeedTests::setUp()
{
   for (int i = 0; i < kNumTests; i++)
   {
      random_string(m_testStrings[i], _countof(m_testStrings[i]));
   }
}

void cHashTableSpeedTests::tearDown()
{
   m_hashTable.Clear();
}


#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

#endif // !NDEBUG (entire file)
