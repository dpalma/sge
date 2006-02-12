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

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMathExpr
//
// A simple class used in hash table tests

enum eMathExprOp { kMEO_Mult, kMEO_Div, kMEO_Add, kMEO_Sub };

eMathExprOp g_mathExprOps[] = { kMEO_Mult, kMEO_Div, kMEO_Add, kMEO_Sub };

template <typename T>
class cMathExpr
{
public:
   cMathExpr()
   {
   }

   cMathExpr(uint operand1, uint operand2, eMathExprOp op)
    : m_operand1(operand1)
    , m_operand2(operand2)
    , m_op(op)
   {
   }

   cMathExpr(const cMathExpr & other)
    : m_operand1(other.m_operand1)
    , m_operand2(other.m_operand2)
    , m_op(other.m_op)
   {
   }

   const cMathExpr & operator =(const cMathExpr & other)
   {
      m_operand1 = other.m_operand1;
      m_operand2 = other.m_operand2;
      m_op = other.m_op;
      return *this;
   }

   bool IsEqual(const cMathExpr & other) const
   {
      return (m_operand1 == other.m_operand1)
         && (m_operand2 == other.m_operand2)
         && (m_op == other.m_op);
   }

   bool operator ==(const cMathExpr & other) const
   {
      return IsEqual(other);
   }

   bool operator !=(const cMathExpr & other) const
   {
      return !IsEqual(other);
   }

   T GetResult() const
   {
      switch (m_op)
      {
      case kMEO_Mult: return m_operand1 * m_operand2;
      case kMEO_Div: return m_operand1 / m_operand2;
      case kMEO_Add: return m_operand1 + m_operand2;
      case kMEO_Sub: return m_operand1 - m_operand2;
      default: return 0;
      }
   }

   static uint Hash(const cMathExpr & k, uint initHash = 0xDEADBEEF)
   {
      uint h = initHash;
      h = hash((byte *)&k.m_operand1, sizeof(k.m_operand1), h);
      h = hash((byte *)&k.m_operand2, sizeof(k.m_operand2), h);
      h = hash((byte *)&k.m_op, sizeof(k.m_op), h);
      return h;
   }

   static bool Equal(const cMathExpr & a, const cMathExpr & b)
   {
      return a.IsEqual(b);
   }

private:
   T m_operand1, m_operand2;
   eMathExprOp m_op;
};


///////////////////////////////////////////////////////////////////////////////

enum
{
   kNumTests = 50,
   kTestStringLength = 20,
};

///////////////////////////////////////////////////////////////////////////////

class cHashTableTests : public CppUnit::TestCase
{
   char m_testStrings[kNumTests][kTestStringLength];

   typedef cHashTable<const char *, int> tTestHashTable;
   tTestHashTable m_hashTable;

   void TestCopy();
   void TestErase();
   void TestFindSuccess();
   void TestFindFailure();
   void TestIterationOfEmptyTable();
   void TestIterationPostIncrement();
   void TestIterationPreIncrement();
   void TestArrayIndexOperator();
   void TestCustomKey();
   void TestCustomValue();

   CPPUNIT_TEST_SUITE(cHashTableTests);
      CPPUNIT_TEST(TestCopy);
      CPPUNIT_TEST(TestErase);
      CPPUNIT_TEST(TestFindSuccess);
      CPPUNIT_TEST(TestFindFailure);
      CPPUNIT_TEST(TestIterationOfEmptyTable);
      CPPUNIT_TEST(TestIterationPostIncrement);
      CPPUNIT_TEST(TestIterationPreIncrement);
      CPPUNIT_TEST(TestArrayIndexOperator);
      CPPUNIT_TEST(TestCustomKey);
      CPPUNIT_TEST(TestCustomValue);
   CPPUNIT_TEST_SUITE_END();

public:
   virtual void setUp();
   virtual void tearDown();
};

////////////////////////////////////////

void cHashTableTests::TestCopy()
{
   tTestHashTable hashTableCopy(m_hashTable);

   CPPUNIT_ASSERT(hashTableCopy.size() == m_hashTable.size());

   tTestHashTable::const_iterator
      iter1 = m_hashTable.begin(),
      iter2 = hashTableCopy.begin();
   for (; iter1 != m_hashTable.end(); iter1++, iter2++)
   {
      const tChar * psz1 = iter1->first;
      const tChar * psz2 = iter2->first;
      CPPUNIT_ASSERT(_tcscmp(psz1, psz2) == 0);
      CPPUNIT_ASSERT(iter1->second == iter2->second);
   }
}

////////////////////////////////////////

void cHashTableTests::TestErase()
{
   uint nErasures = (m_hashTable.size() / 2); // erase half the strings in the table
   std::set<int> erasedIndices;
   while (erasedIndices.size() < nErasures)
   {
      erasedIndices.insert(rand() % kNumTests);
   }

   std::set<int>::const_iterator iter = erasedIndices.begin();
   for (; iter != erasedIndices.end(); iter++)
   {
      const char * psz = m_testStrings[*iter];
      CPPUNIT_ASSERT(m_hashTable.erase(psz) == 1);
   }

   CPPUNIT_ASSERT(m_hashTable.size() == (kNumTests - nErasures));

   for (int i = 0; i < kNumTests; i++)
   {
      tTestHashTable::const_iterator f = m_hashTable.find(m_testStrings[i]);
      if (erasedIndices.find(i) != erasedIndices.end())
      {
         CPPUNIT_ASSERT(f == m_hashTable.end()); // string 'i' should have been erased
      }
      else
      {
         CPPUNIT_ASSERT(f != m_hashTable.end());
      }
   }
}

////////////////////////////////////////

void cHashTableTests::TestFindSuccess()
{
   int nFailures = 0;

   for (int i = 0; i < kNumTests; i++)
   {
      tTestHashTable::const_iterator f = m_hashTable.find(m_testStrings[i]);
      if (f == m_hashTable.end())
      {
         nFailures++;
      }
      else
      {
         CPPUNIT_ASSERT(f->second == i);
      }
   }

   LocalMsgIf1(nFailures > 0, "%d lookups failed\n", nFailures);
   CPPUNIT_ASSERT(nFailures == 0);
}

////////////////////////////////////////

void cHashTableTests::TestFindFailure()
{
   char szNotFound[kTestStringLength + 10];
   random_string(szNotFound, _countof(szNotFound));
   tTestHashTable::const_iterator f = m_hashTable.find(szNotFound);
   CPPUNIT_ASSERT(f == m_hashTable.end());
}

////////////////////////////////////////

void cHashTableTests::TestIterationOfEmptyTable()
{
   cHashTable<int_ptr, void *> hashTable;
   CPPUNIT_ASSERT(hashTable.empty());
   CPPUNIT_ASSERT(hashTable.begin() == hashTable.end());
   cHashTable<int_ptr, void *>::iterator iter = hashTable.begin();
   int nIterated = 0;
   for (; iter != hashTable.end(); iter++, nIterated++)
   {
      CPPUNIT_ASSERT(false); // should never get in here
   }
   CPPUNIT_ASSERT(nIterated == 0);
}

////////////////////////////////////////

void cHashTableTests::TestIterationPostIncrement()
{
   int nIterated = 0;
   tTestHashTable::const_iterator iter = m_hashTable.begin();
   for (; iter != m_hashTable.end(); iter++, nIterated++)
   {
      const char * key = iter->first;
      int val = iter->second;
      CPPUNIT_ASSERT(strcmp(m_testStrings[val], key) == 0);
   }

   LocalMsgIf2(nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CPPUNIT_ASSERT(nIterated == kNumTests);
}

////////////////////////////////////////

void cHashTableTests::TestIterationPreIncrement()
{
   int nIterated = 0;
   tTestHashTable::const_iterator iter = m_hashTable.begin();
   for (; iter != m_hashTable.end(); ++iter, ++nIterated)
   {
      const char * key = iter->first;
      int val = iter->second;
      CPPUNIT_ASSERT(strcmp(m_testStrings[val], key) == 0);
   }

   LocalMsgIf2(nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CPPUNIT_ASSERT(nIterated == kNumTests);
}

////////////////////////////////////////

void cHashTableTests::TestArrayIndexOperator()
{
   // test insertion via array index operator
   tTestHashTable hashTable;
   for (int i = 0; i < kNumTests; i++)
   {
      hashTable[m_testStrings[i]] = i;
   }
   CPPUNIT_ASSERT(hashTable.size() == kNumTests);

   // test value access via array index operator
   for (int i = 0; i < kNumTests; i++)
   {
      int index = hashTable[m_testStrings[i]];
      CPPUNIT_ASSERT(index == i);
   }
   CPPUNIT_ASSERT(hashTable.size() == kNumTests); // make sure size didn't change because of access
}

////////////////////////////////////////

void cHashTableTests::TestCustomKey()
{
   cHashTable<cMathExpr<int>, int, cMathExpr<int> > hashTable;

   for (int i = 0; i < 500; i++)
   {
      int a = rand();
      int b = rand();
      eMathExprOp op = g_mathExprOps[rand() % _countof(g_mathExprOps)];
      while ((op == kMEO_Div) && (b == 0))
      {
         b = rand();
      }
      cMathExpr<int> expr(a,b,op);
      CPPUNIT_ASSERT(hashTable.insert(expr, expr.GetResult()).second);
   }

   cHashTable<cMathExpr<int>, int, cMathExpr<int> >::const_iterator iter = hashTable.begin();
   for (; iter != hashTable.end(); ++iter)
   {
      CPPUNIT_ASSERT(iter->first.GetResult() == iter->second);
   }
}

////////////////////////////////////////

void cHashTableTests::TestCustomValue()
{
   cHashTable<int, cMathExpr<int> > hashTable;

   static const int nTests = 500;

   std::map<int, cMathExpr<int> > exprs;
   while (exprs.size() < nTests)
   {
      int a = rand();
      int b = rand();
      eMathExprOp op = g_mathExprOps[rand() % _countof(g_mathExprOps)];
      while ((op == kMEO_Div) && (b == 0))
      {
         b = rand();
      }
      cMathExpr<int> expr(a,b,op);
      exprs.insert(std::make_pair(expr.GetResult(), expr));
   }

   {
      std::map<int, cMathExpr<int> >::const_iterator iter = exprs.begin();
      for (; iter != exprs.end(); iter++)
      {
         CPPUNIT_ASSERT(hashTable.insert(iter->first, iter->second).second);
      }
   }

   {
      cHashTable<int, cMathExpr<int> >::const_iterator iter = hashTable.begin();
      for (; iter != hashTable.end(); ++iter)
      {
         CPPUNIT_ASSERT(iter->first == iter->second.GetResult());
      }
   }
}

////////////////////////////////////////

void cHashTableTests::setUp()
{
   std::set<std::string> strings;
   while (strings.size() < kNumTests)
   {
      char szTemp[kTestStringLength];
      random_string(szTemp, _countof(szTemp));
      strings.insert(szTemp);
   }

   std::set<std::string>::const_iterator iter = strings.begin();
   for (int index = 0; iter != strings.end(); iter++, index++)
   {
      strcpy(m_testStrings[index], iter->c_str());
      CPPUNIT_ASSERT(m_hashTable.insert(m_testStrings[index], index).second);
   }

   CPPUNIT_ASSERT(m_hashTable.size() == kNumTests);
}

////////////////////////////////////////

void cHashTableTests::tearDown()
{
   m_hashTable.clear();
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

   m_hashTable.clear();
   m_map.clear();

   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CPPUNIT_ASSERT(m_hashTable.insert(m_testStrings[i], i).second);
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
      int64 start = ReadTSC();
      m_hashTable.find(m_testStrings[i]);
      int64 elapsed = ReadTSC() - start;
      *pHashTableResult += (double)(long)elapsed * oneOverNumLookups;
   }

   *pMapResult = 0;
   for (i = 0; i < nLookups; i++)
   {
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
   std::set<std::string> strings;
   while (strings.size() < kNumTests)
   {
      char szTemp[kTestStringLength];
      random_string(szTemp, _countof(szTemp));
      strings.insert(szTemp);
   }
   std::set<std::string>::const_iterator iter = strings.begin();
   for (int index = 0; iter != strings.end(); iter++, index++)
   {
      strcpy(m_testStrings[index], iter->c_str());
   }
}

////////////////////////////////////////

void cHashTableSpeedTests::tearDown()
{
   m_hashTable.clear();
}


#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////

#endif // !NDEBUG (entire file)
