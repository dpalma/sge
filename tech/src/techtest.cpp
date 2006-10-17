///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_UNITTESTPP

extern "C"
{
#include "md5.h"
}

#include "color.h"
#include "digraph.h"
#include "techtime.h"
#include "toposort.h"

#include "UnitTest++.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(TechTest);

#define LocalMsg(msg)                     DebugMsgEx(TechTest,(msg))
#define LocalMsg1(msg,a)                  DebugMsgEx1(TechTest,(msg),(a))
#define LocalMsg2(msg,a,b)                DebugMsgEx2(TechTest,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)              DebugMsgEx3(TechTest,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)            DebugMsgEx4(TechTest,(msg),(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)              DebugMsgIfEx(TechTest,(cond),(msg))
#define LocalMsgIf1(cond,msg,a)           DebugMsgIfEx1(TechTest,(cond),(msg),(a))
#define LocalMsgIf2(cond,msg,a,b)         DebugMsgIfEx2(TechTest,(cond),(msg),(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)       DebugMsgIfEx3(TechTest,(cond),(msg),(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)     DebugMsgIfEx4(TechTest,(cond),(msg),(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////

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
      LogMsgNoFL1(kDebug, "%02x", digest[i]);
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

   LocalMsg2("MD%d (\"%s\") = ", MD, string);
   if (LOG_IS_CHANNEL_ENABLED(TechTest))
   {
      MDPrint(digest);
      LogMsgNoFL(kDebug, "\n");
   }

   return (memcmp(digest, expected, 16 * sizeof(byte)) == 0);
}

////////////////////////////////////////

TEST(MDTimeTrial)
{
   MD_CTX context;
   double endTime, startTime, elapsed;
   byte block[TEST_BLOCK_LEN], digest[16];

   LocalMsg3("MD%d time trial. Digesting %d %d-byte blocks ...", MD, TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

   /* Initialize block */
   int i;
   for (i = 0; i < TEST_BLOCK_LEN; i++)
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

   if (LOG_IS_CHANNEL_ENABLED(TechTest))
   {
      LogMsgNoFL(kDebug, " done\n");
      LocalMsg("Digest = ");
      MDPrint(digest);
      LogMsgNoFL(kDebug, "\n");
   }
   LocalMsg1("Time = %f seconds\n", elapsed);
   LocalMsg1("Speed = %ld bytes/second\n", (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT/elapsed);
}

////////////////////////////////////////

TEST(MDTestSuite)
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
      CHECK(MDString(tests[i].string, tests[i].digest));
   }
}


///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

TEST(DigraphTopoSort)
{
   typedef cDigraph<char, int> tGraph;
   tGraph graph;

   for (int c = 'a'; c <= 'd'; c++)
   {
      graph.insert(c);
   }

   // a -> b -> c
   //   -> d
   CHECK(graph.insert_edge('a', 'b', 1).second);
   CHECK(graph.insert_edge('b', 'c', 1).second);
   CHECK(graph.insert_edge('a', 'd', 1).second);

   CHECK(graph.acyclic());

   std::vector<tGraph::node_type> sorted;
   cTopoSorter<tGraph::node_type> sorter(&sorted);
   graph.topological_sort(sorter);

   CHECK(sorted.size() == 4);

   int positions[4];

   std::vector<tGraph::node_type>::iterator iter;
   for (iter = sorted.begin(); iter != sorted.end(); iter++)
   {
      char c = *iter;
      int i = iter - sorted.begin();
      positions[c - 'a'] = i;
   }

   CHECK(positions['a' - 'a'] < positions['b' - 'a']); // a comes before b
   CHECK(positions['b' - 'a'] < positions['c' - 'a']); // b comes before c
   CHECK(positions['a' - 'a'] < positions['c' - 'a']); // a comes before c (by way of b)
   CHECK(positions['a' - 'a'] < positions['d' - 'a']); // a comes before d
}

////////////////////////////////////////

TEST(DigraphCycleDetect)
{
   typedef cDigraph<char, int> tGraph;
   tGraph graph;

   for (int c = 'a'; c <= 'f'; c++)
   {
      graph.insert(c);
   }

   // The following adds the cycle: a -> d -> f -> a
   CHECK(graph.insert_edge('a', 'b', 0).second);
   CHECK(graph.insert_edge('b', 'c', 0).second);
   CHECK(graph.insert_edge('a', 'd', 0).second);
   CHECK(graph.insert_edge('d', 'f', 0).second);
   CHECK(graph.insert_edge('f', 'a', 0).second);

   CHECK(!graph.acyclic());
}

///////////////////////////////////////////////////////////////////////////////

TEST(ColorCompare)
{
   {
      cColor c1(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
      cColor c2(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
      cColor c3(2.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f);
      CHECK(c1 == c2);
      CHECK(c1 != c3);
   }
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_UNITTESTPP (entire file)
