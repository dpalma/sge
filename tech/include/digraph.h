///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_DIGRAPH_H
#define INCLUDED_DIGRAPH_H

#include <map>

#include "dbgalloc.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cDigraph
//
// Uses adjacency list representation
//
// REFERENCES:
// [1] "Algorithms: Second Edition", Robert Sedgewick, Addison-Wesley. 1988.
// [2] "Introduction to Algorithms", Cormen, Leiserson, & Rivest, MIT Press. 1990.
//

DECLARE_HANDLE(HNODEITER);
DECLARE_HANDLE(HEDGEITER);

template <typename KEY, typename DATA = void *>
class cDigraph
{
   enum eConstants
   {
      kDefaultEdgeWeight = 0
   };

public:
   typedef KEY tKey;
   typedef DATA tData;

   cDigraph();
   ~cDigraph();

   void AddNode(const KEY & key, const DATA & data = DefaultData());

   int CountNodes() const;

   bool AddEdge(const KEY & from, const KEY & to, int weight = kDefaultEdgeWeight);

   bool HasEdge(const KEY & from, const KEY & to) const;

   HNODEITER IterNodesBegin() const;
   bool IterNextNode(HNODEITER hIter, KEY * pKey, DATA * pData = NULL) const;
   void IterNodesEnd(HNODEITER hIter) const;

   HEDGEITER IterEdgesBegin(const KEY & key) const;
   bool IterNextEdge(HEDGEITER hIter, KEY * pTo, int * pWeight = NULL) const;
   void IterEdgesEnd(HEDGEITER hIter) const;

   void SetNodeData(const KEY & key, const DATA & data);
   const DATA & GetNodeData(const KEY & key) const;
   DATA * GetNodeDataPtr(const KEY & key);

private:
   static const DATA & DefaultData();

   ////////////////////////////////////

   typedef std::map<KEY, int> tEdges; // maps dest node to weight/cost to get to that node

   struct sNode
   {
      DATA data;
      tEdges edges; // outgoing edges
   };

   typedef std::map<KEY, sNode *> tNodes;

   struct sEdgeIter
   {
      typename tEdges::const_iterator iter;
      typename tEdges::const_iterator end;
   };

   ////////////////////////////////////

   tNodes m_nodes;
};

///////////////////////////////////////

template <typename KEY, typename DATA>
cDigraph<KEY, DATA>::cDigraph()
{
}

///////////////////////////////////////

template <typename KEY, typename DATA>
cDigraph<KEY, DATA>::~cDigraph()
{
   typename tNodes::iterator iter;
   for (iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
   {
      delete iter->second;
   }
   m_nodes.clear();
}

///////////////////////////////////////

template <typename KEY, typename DATA>
void cDigraph<KEY, DATA>::AddNode(const KEY & key, const DATA & data)
{
   sNode * pNode = new sNode;
   pNode->data = data;
   m_nodes.insert(std::make_pair(key, pNode));
}

///////////////////////////////////////

template <typename KEY, typename DATA>
int cDigraph<KEY, DATA>::CountNodes() const
{
   return m_nodes.size();
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::AddEdge(const KEY & from, const KEY & to, int weight)
{
   typename tNodes::iterator fromIter = m_nodes.find(from);
   if (fromIter != m_nodes.end())
   {
      if (m_nodes.find(to) != m_nodes.end())
      {
#ifdef __GNUC__
         typedef std::pair<typename tEdges::iterator, bool> tInsertResult;
#else
         typedef std::pair<tEdges::iterator, bool> tInsertResult;
#endif
         tInsertResult result = fromIter->second->edges.insert(std::make_pair(to, weight));
         return result.second;
      }
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::HasEdge(const KEY & from, const KEY & to) const
{
   typename tNodes::const_iterator fromIter = m_nodes.find(from);
   if (fromIter != m_nodes.end())
   {
      if (m_nodes.find(to) != m_nodes.end())
      {
         if (fromIter->second->edges.find(to) != fromIter->second->edges.end())
         {
            return true;
         }
      }
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
HNODEITER cDigraph<KEY, DATA>::IterNodesBegin() const
{
   typename tNodes::const_iterator * pIter = new typename tNodes::const_iterator(m_nodes.begin());
   return (HNODEITER)pIter;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::IterNextNode(HNODEITER hIter, KEY * pKey, DATA * pData) const
{
   Assert(hIter != NULL);
   typename tNodes::const_iterator * pIter = (typename tNodes::const_iterator *)hIter;
   if (*pIter != m_nodes.end())
   {
      Assert(pKey != NULL);
      *pKey = (*pIter)->first;
      if (pData != NULL)
         *pData = (*pIter)->second->data;
      (*pIter)++;
      return true;
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
void cDigraph<KEY, DATA>::IterNodesEnd(HNODEITER hIter) const
{
   typename tNodes::const_iterator * pIter = (typename tNodes::const_iterator *)hIter;
   delete pIter;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
HEDGEITER cDigraph<KEY, DATA>::IterEdgesBegin(const KEY & key) const
{
   typename tNodes::const_iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      sEdgeIter * pIter = new sEdgeIter;
      pIter->iter = iter->second->edges.begin();
      pIter->end = iter->second->edges.end();
      return (HEDGEITER)pIter;
   }
   return NULL;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::IterNextEdge(HEDGEITER hIter, KEY * pTo, int * pWeight) const
{
   Assert(hIter != NULL);
   sEdgeIter * pIter = (sEdgeIter *)hIter;
   if (pIter->iter != pIter->end)
   {
      Assert(pTo != NULL);
      *pTo = pIter->iter->first;
      if (pWeight != NULL)
         *pWeight = pIter->iter->second;
      pIter->iter++;
      return true;
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
void cDigraph<KEY, DATA>::IterEdgesEnd(HEDGEITER hIter) const
{
   sEdgeIter * pIter = (sEdgeIter *)hIter;
   delete pIter;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
void cDigraph<KEY, DATA>::SetNodeData(const KEY & key, const DATA & data)
{
   typename tNodes::iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      iter->second->data = data;
   }
}

///////////////////////////////////////

template <typename KEY, typename DATA>
const DATA & cDigraph<KEY, DATA>::GetNodeData(const KEY & key) const
{
   typename tNodes::const_iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      return iter->second->data;
   }
   return DefaultData(); // @HACK: not a very informative error for non-pointer types
}

///////////////////////////////////////

template <typename KEY, typename DATA>
DATA * cDigraph<KEY, DATA>::GetNodeDataPtr(const KEY & key)
{
   typename tNodes::const_iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      return &iter->second->data;
   }
   return NULL;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
const DATA & cDigraph<KEY, DATA>::DefaultData()
{
   // static data is initialized to zero so the default for pointers
   // will be NULL, which is good
   static DATA data;
   return data;
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cDFS
//
// Class for performing a depth-first search on a di-graph

enum eDFSResult
{
   kDFS_OK = 0,
   kDFS_Cycle = -1,
};

template <typename DIGRAPH>
class cDFS
{
public:
   eDFSResult DFS(const DIGRAPH * pGraph)
   {
      eDFSResult result = kDFS_OK;

      tSearchParams searchParams;

      HNODEITER hNodeIter = pGraph->IterNodesBegin();
      Assert(hNodeIter != NULL);

      typename DIGRAPH::tKey key;
      while (pGraph->IterNextNode(hNodeIter, &key))
      {
         result = DFSVisit(pGraph, key, &searchParams);
         if (result != kDFS_OK)
            break;
      }
      pGraph->IterNodesEnd(hNodeIter);

      return result;
   }

protected:
   virtual void OnFinished(const typename DIGRAPH::tKey key)
   {
   }

private:
   enum eSearchStatus
   {
      kStarted,
      kFinished,
   };

   typedef std::map<typename DIGRAPH::tKey, eSearchStatus> tSearchParams;

   eDFSResult DFSVisit(const DIGRAPH * pGraph,
                       const typename DIGRAPH::tKey & key,
                       tSearchParams * pSearchParams)
   {
      typename tSearchParams::iterator iter = pSearchParams->find(key);

      if (iter != pSearchParams->end())
      {
         if (iter->second == kStarted)
         {
            return kDFS_Cycle;
         }
         else if (iter->second == kFinished)
         {
            return kDFS_OK;
         }
         else
         {
            Assert(!"Unknown or invalid node search status");
         }
      }

      (*pSearchParams)[key] = kStarted;

      eDFSResult result = kDFS_OK;

      HEDGEITER hIter = pGraph->IterEdgesBegin(key);
      if (hIter != NULL)
      {
         typename DIGRAPH::tKey to;
         while (pGraph->IterNextEdge(hIter, &to))
         {
            result = DFSVisit(pGraph, to, pSearchParams);
            if (result != kDFS_OK)
               break;
         }
         pGraph->IterEdgesEnd(hIter);
      }

      (*pSearchParams)[key] = kFinished;

      OnFinished(key);

      return result;
   }
};

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_DIGRAPH_H
