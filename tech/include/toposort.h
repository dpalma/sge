///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TOPOSORT_H
#define INCLUDED_TOPOSORT_H

#include <vector>

#include "dbgalloc.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTopoSorter
//

template <class NODE, class CONTAINER = std::vector<NODE> >
class cTopoSorter
{
public:
   cTopoSorter(CONTAINER * pContainer)
    : m_pContainer(pContainer)
   {
   }

   bool operator()(const NODE & node)
   {
      m_pContainer->push_back(node);
      return true;
   }

private:
   CONTAINER * m_pContainer;
};

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_TOPOSORT_H
