///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOPOSORT_H
#define INCLUDED_TOPOSORT_H

#include <vector>
#include "digraph.h"

#include "dbgalloc.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTopoSorter
//

template <class DIGRAPH>
class cTopoSorter : public cDFS<DIGRAPH>
{
public:
   typedef std::vector<typename DIGRAPH::tKey> tSortedArray;

   cTopoSorter();
   ~cTopoSorter();

   void TopoSort(const DIGRAPH * pGraph, tSortedArray * pArray);

   void OnFinished(const typename DIGRAPH::tKey key)
   {
      if (m_pArray != NULL)
         m_pArray->insert(m_pArray->begin(), key);
   }

private:
   tSortedArray * m_pArray;
};

///////////////////////////////////////

template <class DIGRAPH>
cTopoSorter<DIGRAPH>::cTopoSorter() : m_pArray(NULL)
{
}

///////////////////////////////////////

template <class DIGRAPH>
cTopoSorter<DIGRAPH>::~cTopoSorter()
{
}

///////////////////////////////////////

template <class DIGRAPH>
void cTopoSorter<DIGRAPH>::TopoSort(const DIGRAPH * pGraph, tSortedArray * pArray)
{
   m_pArray = pArray;
   m_pArray->clear();

   DFS(pGraph);

   m_pArray = NULL;
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_TOPOSORT_H
