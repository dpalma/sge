///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_CPPUNIT // entire file

extern "C"
{
#include "md5.h"
}

#include "digraph.h"
#include "toposort.h"

#include "techtime.h"

#include <cppunit/extensions/HelperMacros.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMD5Tests
//

class cMD5Tests : public CppUnit::TestCase
{
   void MDTimeTrial();
   void MDTestSuite();

   CPPUNIT_TEST_SUITE(cMD5Tests);
      CPPUNIT_TEST(MDTimeTrial);
      CPPUNIT_TEST(MDTestSuite);
   CPPUNIT_TEST_SUITE_END();
};

////////////////////////////////////////

#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

#define MD 5

#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final

// Prints a message digest in hexadecimal
static void MDPrint(byte digest[16])
{
   for (int i = 0; i < 16; i++)
   {
      techlog.Print(kDebug, "%02x", digest[i]);
   }
}

// Digests a string and prints the result
static bool MDString(char * string, byte expected[16])
{
   MD_CTX context;
   byte digest[16];
   uint len = strlen(string);

   MDInit(&context);
   MDUpdate(&context, reinterpret_cast<byte*>(string), len);
   MDFinal(digest, &context);

   DebugMsg2("MD%d (\"%s\") = ", MD, string);
   MDPrint(digest);
   techlog.Print(kDebug, "\n");

   return (memcmp(digest, expected, 16 * sizeof(byte)) == 0);
}

////////////////////////////////////////

void cMD5Tests::MDTimeTrial()
{
   MD_CTX context;
   double endTime, startTime, elapsed;
   byte block[TEST_BLOCK_LEN], digest[16];

   DebugMsg3("MD%d time trial. Digesting %d %d-byte blocks ...", MD, TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

   /* Initialize block */
   for (int i = 0; i < TEST_BLOCK_LEN; i++)
   {
      block[i] = (byte)(i & 0xff);
   }

   /* Start timer */
   startTime = TimeGetSecs();

   /* Digest blocks */
   MDInit(&context);
   for (i = 0; i < TEST_BLOCK_COUNT; i++)
   {
      MDUpdate (&context, block, TEST_BLOCK_LEN);
   }
   MDFinal(digest, &context);

   /* Stop timer */
   endTime = TimeGetSecs();
   elapsed = endTime - startTime;

   techlog.Print(kDebug, " done\n");
   DebugMsg("Digest = ");
   MDPrint(digest);
   techlog.Print(kDebug, "\n");
   DebugMsg1("Time = %f seconds\n", elapsed);
   DebugMsg1("Speed = %ld bytes/second\n", (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT/elapsed);
}

////////////////////////////////////////

void cMD5Tests::MDTestSuite()
{
   static struct
   {
      char * string;
      byte digest[16];
   }
   tests[] =
   {
      { "", { 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e } },
      { "a", { 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 } },
      { "abc", { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 } },
      { "message digest", { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 } },
      { "abcdefghijklmnopqrstuvwxyz", { 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b } },
      { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", { 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f } },
      { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", { 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a } },
   };
   for (int i = 0; i < _countof(tests); i++)
   {
      CPPUNIT_ASSERT(MDString(tests[i].string, tests[i].digest));
   }
}

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cMD5Tests);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDigraphTests
//

class cDigraphTests : public CppUnit::TestCase
{
   void TestTopoSort();
   void TestCycleDetect();

   CPPUNIT_TEST_SUITE(cDigraphTests);
      CPPUNIT_TEST(TestTopoSort);
      CPPUNIT_TEST(TestCycleDetect);
   CPPUNIT_TEST_SUITE_END();
};

////////////////////////////////////////

void cDigraphTests::TestTopoSort()
{
   typedef cDigraph<char> tGraph;
   tGraph graph;

   for (int c = 'a'; c <= 'd'; c++)
   {
      graph.AddNode(c);
   }

   CPPUNIT_ASSERT(graph.AddEdge('a', 'b'));
   CPPUNIT_ASSERT(graph.AddEdge('b', 'c'));
   CPPUNIT_ASSERT(graph.AddEdge('a', 'd'));

   std::vector<tGraph::tKey> sorted;

   cTopoSorter<tGraph> sorter;
   sorter.TopoSort(&graph, &sorted);

   const char answer[] = "adbc\0";

   CPPUNIT_ASSERT(sorted.size() == strlen(answer));

   std::vector<tGraph::tKey>::iterator iter;
   for (iter = sorted.begin(); iter != sorted.end(); iter++)
   {
      CPPUNIT_ASSERT((*iter) == answer[iter - sorted.begin()]);
   }
}

////////////////////////////////////////

void cDigraphTests::TestCycleDetect()
{
   typedef cDigraph<char> tGraph;
   tGraph graph;

   for (int c = 'a'; c <= 'f'; c++)
   {
      graph.AddNode(c);
   }

   CPPUNIT_ASSERT(graph.AddEdge('a', 'b'));
   CPPUNIT_ASSERT(graph.AddEdge('b', 'c'));
   CPPUNIT_ASSERT(graph.AddEdge('a', 'd'));
   CPPUNIT_ASSERT(graph.AddEdge('d', 'f'));
   CPPUNIT_ASSERT(graph.AddEdge('f', 'a'));

   cDFS<tGraph> dfs;
   CPPUNIT_ASSERT(dfs.DFS(&graph) == kDFS_Cycle);
}

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cDigraphTests);

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT (entire file)
