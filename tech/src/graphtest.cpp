///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_CPPUNIT // entire file

#include "digraph.h"
#include "toposort.h"
#include <cppunit/extensions/HelperMacros.h>

#include <dbgalloc.h> // must be last header

///////////////////////////////////////////////////////////////////////////////

class cDigraphTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cDigraphTests);
      CPPUNIT_TEST(TestTopoSort);
      CPPUNIT_TEST(TestCycleDetect);
   CPPUNIT_TEST_SUITE_END();

public:
   void TestTopoSort()
   {
      typedef cDigraph<char> tGraph;
      tGraph graph;

      for (int c = 'a'; c <= 'd'; c++)
         graph.AddNode(c);

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

   void TestCycleDetect()
   {
      typedef cDigraph<char> tGraph;
      tGraph graph;

      for (int c = 'a'; c <= 'f'; c++)
         graph.AddNode(c);

      CPPUNIT_ASSERT(graph.AddEdge('a', 'b'));
      CPPUNIT_ASSERT(graph.AddEdge('b', 'c'));
      CPPUNIT_ASSERT(graph.AddEdge('a', 'd'));
      CPPUNIT_ASSERT(graph.AddEdge('d', 'f'));
      CPPUNIT_ASSERT(graph.AddEdge('f', 'a'));

      cDFS<tGraph> dfs;
      CPPUNIT_ASSERT(dfs.DFS(&graph) == kDFS_Cycle);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cDigraphTests);

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT (entire file)
