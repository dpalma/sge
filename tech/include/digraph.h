//////////////////////////////////////////////////////////////////////////////
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
   typedef int tWeight;

   cDigraph();
   ~cDigraph();

   bool AddNode(const KEY & key, const DATA & data = DefaultData());
   bool HasNode(const KEY & key) const;
   int CountNodes() const;

   bool AddEdge(const KEY & from, const KEY & to, tWeight weight = kDefaultEdgeWeight);
   bool HasEdge(const KEY & from, const KEY & to) const;

   HNODEITER IterNodesBegin() const;
   bool IterNextNode(HNODEITER hIter, KEY * pKey, DATA * pData = NULL) const;
   void IterNodesEnd(HNODEITER hIter) const;

   HEDGEITER IterEdgesBegin(const KEY & key) const;
   bool IterNextEdge(HEDGEITER hIter, KEY * pTo, tWeight * pWeight = NULL) const;
   void IterEdgesEnd(HEDGEITER hIter) const;

   bool SetNodeData(const KEY & key, const DATA & data);
   bool GetNodeData(const KEY & key, DATA * pData) const;
   DATA * GetNodeDataPtr(const KEY & key);
   const DATA * GetNodeDataPtr(const KEY & key) const;

private:
   static const DATA & DefaultData();

   ////////////////////////////////////

   typedef std::map<KEY, tWeight> tEdges; // maps dest node to weight/cost to get to that node

   typedef std::pair<typename tEdges::iterator, bool> tEdgeInsertResult;

   struct sNode
   {
      DATA data;
      tEdges edges; // outgoing edges
   };

   typedef std::map<KEY, sNode *> tNodes;

   typedef std::pair<typename tNodes::iterator, bool> tNodeInsertResult;

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
bool cDigraph<KEY, DATA>::AddNode(const KEY & key, const DATA & data)
{
   if (!HasNode(key))
   {
      sNode * pNode = new sNode;
      pNode->data = data;
      tNodeInsertResult result = m_nodes.insert(std::make_pair(key, pNode));
      return result.second;
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::HasNode(const KEY & key) const
{
   typename tNodes::const_iterator iter = m_nodes.find(key);
   return (iter != m_nodes.end());
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
      if (HasNode(to))
      {
         tEdgeInsertResult result = fromIter->second->edges.insert(std::make_pair(to, weight));
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
   if ((fromIter != m_nodes.end()) && HasNode(to))
   {
      if (fromIter->second->edges.find(to) != fromIter->second->edges.end())
      {
         return true;
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
bool cDigraph<KEY, DATA>::SetNodeData(const KEY & key, const DATA & data)
{
   typename tNodes::iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      iter->second->data = data;
      return true;
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
bool cDigraph<KEY, DATA>::GetNodeData(const KEY & key, DATA * pData) const
{
   typename tNodes::const_iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      if (pData != NULL)
      {
         *pData = iter->second->data;
      }
      return true;
   }
   return false;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
DATA * cDigraph<KEY, DATA>::GetNodeDataPtr(const KEY & key)
{
   typename tNodes::iterator iter = m_nodes.find(key);
   if (iter != m_nodes.end())
   {
      return &iter->second->data;
   }
   return NULL;
}

///////////////////////////////////////

template <typename KEY, typename DATA>
const DATA * cDigraph<KEY, DATA>::GetNodeDataPtr(const KEY & key) const
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
