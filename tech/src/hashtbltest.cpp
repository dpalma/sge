///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_UNITTESTPP // entire file

#include "hashtable.h"
#include "hashtabletem.h"

#include "techtime.h"

#include "UnitTest++.h"

#if (_MSC_VER >= 1310)
#define HASH_MAP_NS stdext
#define HAVE_HASH_MAP 1
#include <hash_map>
#elif (_MSC_VER >= 1200)
#define HASH_MAP_NS std
#define HAVE_HASH_MAP 1
#include <hash_map>
#else
#define HAVE_HASH_MAP 0
#endif

#include <map>
#include <set>
#include <string>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

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

static cRand g_rand((uint)((ReadTSC() >> 32) & 0xFFFFFFFF));

static bool coin_flip(void)
{
   return ((g_rand.Next() & 1) == 0) ? true : false;
}

static void random_string(char * str, int maxlen)
{
   if (maxlen <= 3)
      return;

   int len = 3 + (g_rand.Next() % (maxlen - 3));

   for (int i = 0; i < len; i++)
   {
      if (coin_flip())
         str[i] = 'a' + (g_rand.Next() % 26);
      else
         str[i] = 'A' + (g_rand.Next() % 26);
   }

   str[len - 1] = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMathExpr
//
// A simple class used in hash table tests

enum eMathExprOp { kMEO_Mult, kMEO_Div, kMEO_Add, kMEO_Sub };

static const eMathExprOp g_mathExprOps[] = { kMEO_Mult, kMEO_Div, kMEO_Add, kMEO_Sub };

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

   ~cMathExpr()
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
//
// CLASS: cAllocatorForTesting
//

template <class T>
class cAllocatorForTesting
{
public:
   typedef size_t    size_type;
   typedef ptrdiff_t difference_type;
   typedef T *       pointer;
   typedef const T * const_pointer;
   typedef T &       reference;
   typedef const T & const_reference;
   typedef T         value_type;
   template <class U> struct rebind { typedef cAllocatorForTesting<U> other; };

   cAllocatorForTesting() throw() {}
   cAllocatorForTesting(const cAllocatorForTesting &) throw() {}
   template <class U> cAllocatorForTesting(const cAllocatorForTesting<U> &) throw() {}
   ~cAllocatorForTesting() throw() {}

   pointer address(reference x) const { return &x; }
   const_pointer address(const_reference x) const { return &x; }

   pointer allocate(size_type n, const_pointer hint = NULL)
   {
      void * pMem = malloc(sizeof(T) * n);
      if (pMem == NULL)
         throw std::bad_alloc();
      return static_cast<pointer>(pMem);
   }

   void deallocate(pointer p, size_type)
   {
      if (p != NULL)
         free(p);
   }

   size_type max_size() const throw() { return ~0u / sizeof(T); }

   void construct(pointer p, const T & val)
   {
#ifdef DBGALLOC_MAPPED
#undef new
      new(p) T(val);
#define new DebugNew
#else
      new(p) T(val);
#endif
   }

   void destroy(pointer p)
   {
      p->~T();
   }
};

template <class T1, class T2>
bool operator ==(const cAllocatorForTesting<T1> &, const cAllocatorForTesting<T2> &) throw()
{
   return true;
}

template <class T1, class T2>
bool operator !=(const cAllocatorForTesting<T1> &, const cAllocatorForTesting<T2> &) throw()
{
   return false;
}

template <>
class cAllocatorForTesting<void>
{
public:
   typedef void * pointer;
   typedef const void * const_pointer;
   typedef void value_type;
   template <class U> struct rebind { typedef cAllocatorForTesting<U> other; };
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cHashTableTests
//

enum
{
   kNumTests = 50,
   kTestStringLength = 20,
};

class cHashTableTests
{
public:
   cHashTableTests();
   ~cHashTableTests();

   char m_testStrings[kNumTests][kTestStringLength];

   typedef cHashTable<const char *, int> tTestHashTable;
   tTestHashTable m_hashTable;
};

////////////////////////////////////////

cHashTableTests::cHashTableTests()
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
      Assert(m_hashTable.insert(m_testStrings[index], index).second);
   }

   Assert(m_hashTable.size() == kNumTests);
}

////////////////////////////////////////

cHashTableTests::~cHashTableTests()
{
   m_hashTable.clear();
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestCopy)
{
   tTestHashTable hashTableCopy(m_hashTable);

   CHECK(hashTableCopy.size() == m_hashTable.size());

   tTestHashTable::const_iterator
      iter1 = m_hashTable.begin(),
      iter2 = hashTableCopy.begin();
   for (; iter1 != m_hashTable.end(); iter1++, iter2++)
   {
      const tChar * psz1 = iter1->first;
      const tChar * psz2 = iter2->first;
      CHECK(_tcscmp(psz1, psz2) == 0);
      CHECK(iter1->second == iter2->second);
   }
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestErase)
{
   uint nErasures = (m_hashTable.size() / 2); // erase half the strings in the table
   std::set<int> erasedIndices;
   while (erasedIndices.size() < nErasures)
   {
      erasedIndices.insert(g_rand.Next() % kNumTests);
   }

   std::set<int>::const_iterator iter = erasedIndices.begin();
   for (; iter != erasedIndices.end(); iter++)
   {
      const char * psz = m_testStrings[*iter];
      CHECK(m_hashTable.erase(psz) == 1);
   }

   CHECK(m_hashTable.size() == (kNumTests - nErasures));

   for (int i = 0; i < kNumTests; i++)
   {
      tTestHashTable::const_iterator f = m_hashTable.find(m_testStrings[i]);
      if (erasedIndices.find(i) != erasedIndices.end())
      {
         CHECK(f == m_hashTable.end()); // string 'i' should have been erased
      }
      else
      {
         CHECK(f != m_hashTable.end());
      }
   }
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestFindSuccess)
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
         CHECK(f->second == i);
      }
   }

   LocalMsgIf1(nFailures > 0, "%d lookups failed\n", nFailures);
   CHECK(nFailures == 0);
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestFindFailure)
{
   char szNotFound[kTestStringLength + 10];
   random_string(szNotFound, _countof(szNotFound));
   tTestHashTable::const_iterator f = m_hashTable.find(szNotFound);
   CHECK(f == m_hashTable.end());
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestIterationOfEmptyTable)
{
   cHashTable<int_ptr, void *> hashTable;
   CHECK(hashTable.empty());
   CHECK(hashTable.begin() == hashTable.end());
   cHashTable<int_ptr, void *>::iterator iter = hashTable.begin();
   int nIterated = 0;
   for (; iter != hashTable.end(); iter++, nIterated++)
   {
      CHECK(false); // should never get in here
   }
   CHECK(nIterated == 0);
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestIterationPostIncrement)
{
   int nIterated = 0;
   tTestHashTable::const_iterator iter = m_hashTable.begin();
   for (; iter != m_hashTable.end(); iter++, nIterated++)
   {
      const char * key = iter->first;
      int val = iter->second;
      CHECK(strcmp(m_testStrings[val], key) == 0);
   }

   LocalMsgIf2(nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CHECK(nIterated == kNumTests);
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestIterationPreIncrement)
{
   int nIterated = 0;
   tTestHashTable::const_iterator iter = m_hashTable.begin();
   for (; iter != m_hashTable.end(); ++iter, ++nIterated)
   {
      const char * key = iter->first;
      int val = iter->second;
      CHECK(strcmp(m_testStrings[val], key) == 0);
   }

   LocalMsgIf2(nIterated != kNumTests, "Iterated %d items in hash table; expected %d\n", nIterated, kNumTests);
   CHECK(nIterated == kNumTests);
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableTests, TestArrayIndexOperator)
{
   // test insertion via array index operator
   tTestHashTable hashTable;
   for (int i = 0; i < kNumTests; i++)
   {
      hashTable[m_testStrings[i]] = i;
   }
   CHECK(hashTable.size() == kNumTests);

   // test value access via array index operator
   for (int i = 0; i < kNumTests; i++)
   {
      int index = hashTable[m_testStrings[i]];
      CHECK(index == i);
   }
   CHECK(hashTable.size() == kNumTests); // make sure size didn't change because of access
}

////////////////////////////////////////

TEST(HashTableCustomKey)
{
   cHashTable<cMathExpr<int>, int, cMathExpr<int> > hashTable;

   for (int i = 0; i < 500; i++)
   {
      int a = g_rand.Next();
      int b = g_rand.Next();
      eMathExprOp op = g_mathExprOps[rand() % _countof(g_mathExprOps)];
      while ((op == kMEO_Div) && (b == 0))
      {
         b = g_rand.Next();
      }
      cMathExpr<int> expr(a,b,op);
      CHECK(hashTable.insert(expr, expr.GetResult()).second);
   }

   cHashTable<cMathExpr<int>, int, cMathExpr<int> >::const_iterator iter = hashTable.begin();
   for (; iter != hashTable.end(); ++iter)
   {
      CHECK(iter->first.GetResult() == iter->second);
   }

   sHashTableStats stats;
   hashTable.collect_stats(&stats);
}

////////////////////////////////////////

TEST(HashTableCustomValue)
{
   cHashTable<int, cMathExpr<int> > hashTable;

   static const int nTests = 500;

   std::map<int, cMathExpr<int> > exprs;
   while (exprs.size() < nTests)
   {
      int a = g_rand.Next();
      int b = g_rand.Next();
      eMathExprOp op = g_mathExprOps[rand() % _countof(g_mathExprOps)];
      while ((op == kMEO_Div) && (b == 0))
      {
         b = g_rand.Next();
      }
      cMathExpr<int> expr(a,b,op);
      exprs.insert(std::make_pair(expr.GetResult(), expr));
   }

   {
      std::map<int, cMathExpr<int> >::const_iterator iter = exprs.begin();
      for (; iter != exprs.end(); iter++)
      {
         CHECK(hashTable.insert(iter->first, iter->second).second);
      }
   }

   {
      cHashTable<int, cMathExpr<int> >::const_iterator iter = hashTable.begin();
      for (; iter != hashTable.end(); ++iter)
      {
         CHECK(iter->first == iter->second.GetResult());
      }
   }
}

////////////////////////////////////////

TEST(HashTableCustomAllocator)
{
   typedef cAllocatorForTesting< sHashElement<cMathExpr<int>, int> > tAlloc;
   tAlloc alloc;
   cHashTable<cMathExpr<int>, int, cMathExpr<int>, tAlloc> hashTable(16, alloc);

   for (int i = 0; i < 500; i++)
   {
      int a = g_rand.Next();
      int b = g_rand.Next();
      eMathExprOp op = g_mathExprOps[rand() % _countof(g_mathExprOps)];
      while ((op == kMEO_Div) && (b == 0))
      {
         b = g_rand.Next();
      }
      cMathExpr<int> expr(a,b,op);
      CHECK(hashTable.insert(expr, expr.GetResult()).second);
   }

   cHashTable<cMathExpr<int>, int, cMathExpr<int> >::const_iterator iter = hashTable.begin();
   for (; iter != hashTable.end(); ++iter)
   {
      CHECK(iter->first.GetResult() == iter->second);
   }
}

///////////////////////////////////////////////////////////////////////////////

struct sTiming
{
   int64 clockTicks;
   double seconds;
};

class cHashTableSpeedTests
{
public:
   cHashTableSpeedTests();
   ~cHashTableSpeedTests();

   char m_testStrings[kNumTests][kTestStringLength];

   cHashTable<const char *, int> m_hashTable;
   std::map<const char *, int> m_map;
#if HAVE_HASH_MAP
   HASH_MAP_NS::hash_map<const char *, int> m_hashMap;
#endif

   void RunInsertSpeedTest(sTiming * pHashTableResult, sTiming * pMapResult, sTiming * pHashMapResult, UnitTest::TestResults & testResults_, const char * m_name);
   void TestInsertSpeed();
   void RunLookupSpeedTest(int nLookups, double * pHashTableResult, double * pMapResult, double * pHashMapResult, UnitTest::TestResults & testResults_, const char * m_name);
   void TestLookupSpeed();
};

////////////////////////////////////////

cHashTableSpeedTests::cHashTableSpeedTests()
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

cHashTableSpeedTests::~cHashTableSpeedTests()
{
   m_hashTable.clear();
}

////////////////////////////////////////

void cHashTableSpeedTests::RunInsertSpeedTest(sTiming * pHashTableResult,
                                              sTiming * pMapResult,
                                              sTiming * pHashMapResult,
                                              UnitTest::TestResults & testResults_,
                                              const char * m_testName)
{
   CHECK(pHashTableResult != NULL);
   CHECK(pMapResult != NULL);

   int64 startTicks;
   double startSecs;

   m_hashTable.clear();
   m_map.clear();
#if HAVE_HASH_MAP
   m_hashMap.clear();
#endif

   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CHECK(m_hashTable.insert(m_testStrings[i], i).second);
      }

      pHashTableResult->clockTicks = ReadTSC() - startTicks;
      pHashTableResult->seconds = TimeGetSecs() - startSecs;
   }

   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CHECK(m_map.insert(std::make_pair((const char *)m_testStrings[i], i)).second);
      }

      pMapResult->clockTicks = ReadTSC() - startTicks;
      pMapResult->seconds = TimeGetSecs() - startSecs;
   }

#if HAVE_HASH_MAP
   {
      startTicks = ReadTSC();
      startSecs = TimeGetSecs();

      for (int i = 0; i < kNumTests; i++)
      {
         CHECK(m_hashMap.insert(std::make_pair((const char *)m_testStrings[i], i)).second);
      }

      pHashMapResult->clockTicks = ReadTSC() - startTicks;
      pHashMapResult->seconds = TimeGetSecs() - startSecs;
   }
#endif
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableSpeedTests, TestInsertSpeed)
{
   const int kNumRuns = 10;
   const double kOneOverNumRuns = 1.0 / kNumRuns;

   sTiming hashTableResult[kNumRuns], mapResult[kNumRuns], hashMapResult[kNumRuns];
   double hashTableAverageTicks = 0, mapAverageTicks = 0, hashMapAverageTicks = 0;

   LocalMsg2("Insert Speed Test; inserting %d items; %d runs\n", kNumTests, kNumRuns);
   for (int i = 0; i < kNumRuns; i++)
   {
      RunInsertSpeedTest(&hashTableResult[i], &mapResult[i], &hashMapResult[i], testResults_, m_testName);

      LocalMsg3("   [%d] cHashTable:      %.5f seconds, %d clock ticks\n",
         i, hashTableResult[i].seconds, hashTableResult[i].clockTicks);

      LocalMsg3("   [%d] std::map:        %.5f seconds, %d  clock ticks\n",
         i, mapResult[i].seconds, mapResult[i].clockTicks);

#if HAVE_HASH_MAP
      LocalMsg3("   [%d] std::hash_map:   %.5f seconds, %d  clock ticks\n",
         i, hashMapResult[i].seconds, hashMapResult[i].clockTicks);
#endif

      hashTableAverageTicks += (double)(long)hashTableResult[i].clockTicks * kOneOverNumRuns;
      mapAverageTicks += (double)(long)mapResult[i].clockTicks * kOneOverNumRuns;
#if HAVE_HASH_MAP
      hashMapAverageTicks += (double)(long)hashMapResult[i].clockTicks * kOneOverNumRuns;
#endif
   }

   LocalMsg2("Inserting %d items (average over %d runs):\n", kNumTests, kNumRuns);
   LocalMsg1("   cHashTable:     %.2f clock ticks\n", hashTableAverageTicks);
   LocalMsg1("   std::map:       %.2f clock ticks\n", mapAverageTicks);
#if HAVE_HASH_MAP
   LocalMsg1("   std::hash_map:  %.2f clock ticks\n", hashMapAverageTicks);
#endif
}

////////////////////////////////////////

void cHashTableSpeedTests::RunLookupSpeedTest(int nLookups,
                                              double * pHashTableResult,
                                              double * pMapResult,
                                              double * pHashMapResult,
                                              UnitTest::TestResults & testResults_,
                                              const char * m_testName)
{
   CHECK(pHashTableResult != NULL);
   CHECK(pMapResult != NULL);
   CHECK(pHashMapResult != NULL);

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

   *pHashMapResult = 0;
#if HAVE_HASH_MAP
   for (i = 0; i < nLookups; i++)
   {
      int64 start = ReadTSC();
      m_hashMap.find(m_testStrings[i]);
      int64 elapsed = ReadTSC() - start;
      *pHashMapResult += (double)(long)elapsed * oneOverNumLookups;
   }
#endif
}

////////////////////////////////////////

TEST_FIXTURE(cHashTableSpeedTests, TestLookupSpeed)
{
   double hashTableResult, mapResult, hashMapResult;
   RunLookupSpeedTest(kNumTests, &hashTableResult, &mapResult, &hashMapResult, testResults_, m_testName);

   LocalMsg1("Lookup (average over %d lookups):\n", kNumTests);
   LocalMsg1("   cHashTable:     %.2f clock ticks\n", hashTableResult);
   LocalMsg1("   std::map:       %.2f clock ticks\n", mapResult);
#if HAVE_HASH_MAP
   LocalMsg1("   std::hash_map:  %.2f clock ticks\n", hashMapResult);
#endif
}


///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP (entire file)
