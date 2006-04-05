//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DIGRAPH_H
#define INCLUDED_DIGRAPH_H

#include <map>
#include <queue>
#include <set>
#include <stack>

#include "dbgalloc.h"

#ifdef _MSC_VER
#pragma once
#endif

// REFERENCES
// "Algorithms: Second Edition", Robert Sedgewick, Addison-Wesley. 1988.
// "Introduction to Algorithms", Cormen, Leiserson, & Rivest, MIT Press. 1990.
// http://www.brpreiss.com/books/opus5/html/page556.html#SECTION0017331000000000000000


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cDigraph
//

template <typename NODE, typename EDGE,
          typename NODECOMP = std::less<NODE>,
          typename ALLOC = std::allocator<NODE> >
class cDigraph
{
public:
   typedef size_t size_type;
   typedef NODE node_type;
   typedef typename ALLOC::template rebind<node_type>::other node_allocator;
   typedef NODECOMP node_comp;
   typedef std::set<node_type, node_comp, node_allocator> tNodeSet;
   typedef typename tNodeSet::iterator node_iterator;
   typedef typename tNodeSet::const_iterator const_node_iterator;
   struct sEdge
   {
      const_node_iterator from;
      const_node_iterator to;
      EDGE data;
   };
   typedef struct sEdge edge_type;
   typedef typename ALLOC::template rebind<edge_type>::other edge_allocator;
   struct sEdgeComp
   {
      bool operator()(const edge_type & edge1, const edge_type & edge2) const
      {
         // Sort by from-node, then by to-node (edge data doesn't factor into sort order)
         node_comp nodeComp;
         // If edge1.from < edge2.from (for example)
         if (nodeComp(*edge1.from, *edge2.from))
         {
            return true;
         }
         // If not edge2.from < edge1.from then the from parts are equal so compare to parts
         if (!nodeComp(*edge2.from, *edge1.from) && nodeComp(*edge1.to, *edge2.to))
         {
            return true;
         }
         return false;
      }
   };
   typedef struct sEdgeComp edge_comp;
   typedef std::multiset<edge_type, edge_comp, edge_allocator> tEdgeSet;
   typedef typename tEdgeSet::iterator edge_iterator;
   typedef typename tEdgeSet::const_iterator const_edge_iterator;

   cDigraph();
   ~cDigraph();

   std::pair<node_iterator, bool> insert(const NODE & node);

   node_iterator find(const NODE & node);
   const_node_iterator find(const NODE & node) const;

   node_iterator begin();
   node_iterator end();
   const_node_iterator begin() const;
   const_node_iterator end() const;
   
   size_type size() const;

   std::pair<edge_iterator, bool> insert_edge(const node_type & from, const node_type & to, const EDGE & data);

   /*
   template <typename VISITOR>
   void depth_first_traverse(VISITOR visitor) const
   {
      if (m_edgeSet.empty())
      {
         return;
      }

      std::set<const_node_iterator> visited;
      std::stack<const_edge_iterator> s;
      s.push(m_edgeSet.begin());
      while (!s.empty())
      {
         const_edge_iterator edge = s.top();
         s.pop();

         if (!visitor(*edge))
         {
            return;
         }

         const_edge_iterator iter = m_edgeSet.lower_bound(*edge);
         const_edge_iterator end = m_edgeSet.upper_bound(*edge);
         for (; iter != end; iter++)
         {
            if (visited.find(*iter.from) == visited.end())
            {
               visited.insert(iter.from);
               s.push(iter);
            }
         }
      }
   }
   */

   template <typename VISITOR>
   void topological_sort(VISITOR visitor) const
   {
#ifdef __GNUC__
      std::map<node_type, int> inDegrees;
#else
      std::map<node_type, int, node_comp, node_allocator> inDegrees;
#endif

      {
         const_node_iterator iter = begin();
         for (; iter != end(); iter++)
         {
            inDegrees[*iter] = 0;
         }
      }

      {
         const_edge_iterator iter = m_edgeSet.begin();
         for (; iter != m_edgeSet.end(); iter++)
         {
            inDegrees[*iter->to] += 1;
         }
      }

      std::queue<node_type> q;

      {
         const_node_iterator iter = begin();
         for (; iter != end(); iter++)
         {
            if (inDegrees[*iter] == 0)
            {
               q.push(*iter);
            }
         }
      }

      node_comp nodeComp;

      while (!q.empty())
      {
         node_type node = q.front();
         q.pop();
         if (!visitor(node))
         {
            return;
         }
         const_edge_iterator iter = m_edgeSet.begin();
         for (; iter != m_edgeSet.end(); iter++)
         {
            // Equality test implemented as !(a < b) && !(b < a) in order
            // to use the node_comp template parameter
            if (!nodeComp(*iter->from, node) && !nodeComp(node, *iter->from))
            {
               if (--inDegrees[*iter->to] == 0)
               {
                  q.push(*iter->to);
               }
            }
         }
      }
   }

   bool acyclic() const;

private:
   enum { White, Gray, Black };
   bool acyclic_visit(const node_type & node, std::map<node_type, byte, node_comp, node_allocator> * pColors) const;

   tNodeSet m_nodeSet;
   tEdgeSet m_edgeSet;
};

///////////////////////////////////////

#define DIGRAPH_TEMPLATE_DECL \
   template <typename NODE, typename EDGE, typename NODECOMP, typename ALLOC>
#define DIGRAPH_TEMPLATE_CLASS \
   cDigraph<NODE, EDGE, NODECOMP, ALLOC>

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
DIGRAPH_TEMPLATE_CLASS::cDigraph()
{
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
DIGRAPH_TEMPLATE_CLASS::~cDigraph()
{
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
std::pair<typename DIGRAPH_TEMPLATE_CLASS::node_iterator, bool> DIGRAPH_TEMPLATE_CLASS::insert(const node_type & node)
{
   return m_nodeSet.insert(node);
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::node_iterator DIGRAPH_TEMPLATE_CLASS::find(const node_type & node)
{
   return m_nodeSet.find(node);
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::const_node_iterator DIGRAPH_TEMPLATE_CLASS::find(const node_type & node) const
{
   return m_nodeSet.find(node);
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::node_iterator DIGRAPH_TEMPLATE_CLASS::begin()
{
   return m_nodeSet.begin();
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::node_iterator DIGRAPH_TEMPLATE_CLASS::end()
{
   return m_nodeSet.end();
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::const_node_iterator DIGRAPH_TEMPLATE_CLASS::begin() const
{
   return m_nodeSet.begin();
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::const_node_iterator DIGRAPH_TEMPLATE_CLASS::end() const
{
   return m_nodeSet.end();
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
typename DIGRAPH_TEMPLATE_CLASS::size_type DIGRAPH_TEMPLATE_CLASS::size() const
{
   return m_nodeSet.size();
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
std::pair<typename DIGRAPH_TEMPLATE_CLASS::edge_iterator, bool>
DIGRAPH_TEMPLATE_CLASS::insert_edge(const node_type & from,
                                    const node_type & to,
                                    const EDGE & data)
{
   const_node_iterator fromIter = find(from);
   if (fromIter != end())
   {
      const_node_iterator toIter = find(to);
      if (toIter != end())
      {
         edge_type edge;
         edge.from = fromIter;
         edge.to = toIter;
         edge.data = data;
         edge_iterator edgeIter = m_edgeSet.find(edge);
         if (edgeIter == m_edgeSet.end())
         {
            return std::make_pair(m_edgeSet.insert(edge), true);
         }
         else
         {
            return std::make_pair(edgeIter, false);
         }
      }
   }
   return std::make_pair(m_edgeSet.end(), false);
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
bool DIGRAPH_TEMPLATE_CLASS::acyclic() const
{
   std::map<node_type, byte, node_comp, node_allocator> color;

   {
      const_node_iterator iter = m_nodeSet.begin();
      for (; iter != m_nodeSet.end(); iter++)
      {
         color.insert(std::make_pair(*iter, (byte)White));
      }
   }

   {
      const_node_iterator iter = m_nodeSet.begin();
      for (; iter != m_nodeSet.end(); iter++)
      {
         if (color[*iter] == White)
         {
            if (!acyclic_visit(*iter, &color))
            {
               return false; // found cycle so return false (not acyclic)
            }
         }
      }
   }

   return true;
}

///////////////////////////////////////

DIGRAPH_TEMPLATE_DECL
bool DIGRAPH_TEMPLATE_CLASS::acyclic_visit(const node_type & node,
                                           std::map<node_type, byte, node_comp, node_allocator> * pColors) const
{
   node_comp nodeComp;
   (*pColors)[node] = Gray;
   const_edge_iterator edgeIter = m_edgeSet.begin();
   const_edge_iterator edgeEnd = m_edgeSet.end();
   for (; edgeIter != edgeEnd; edgeIter++)
   {
      if (nodeComp(node, *edgeIter->from) || nodeComp(*edgeIter->from, node))
      {
         continue;
      }
      byte b = (*pColors)[*edgeIter->to];
      if (b == Gray)
      {
         return false; // cycle found (i.e., not acyclic)
      }
      else if (b == White)
      {
         if (!acyclic_visit(*edgeIter->to, pColors))
         {
            return false;
         }
      }
   }
   (*pColors)[node] = Black;
   return true;
}


///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_DIGRAPH_H
