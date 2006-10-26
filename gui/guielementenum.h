///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTENUM_H
#define INCLUDED_GUIELEMENTENUM_H

#include "gui/guiapi.h"
#include "gui/guielementapi.h"

#include "tech/comenum.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef std::list<IGUIElement *> tGUIElementList;
tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum);


///////////////////////////////////////////////////////////////////////////////

template <typename F>
ulong ForEach(IGUIElementEnum * pEnum, F f)
{
   IGUIElement * pElements[32];
   ulong count = 0, total = 0;
   while (SUCCEEDED((pEnum->Next(_countof(pElements), &pElements[0], &count))) && (count > 0))
   {
      for (ulong i = 0; i < count; i++)
      {
         f(pElements[i]);
         SafeRelease(pElements[i]);
      }
      total += count;
      count = 0;
   }
   return total;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTENUM_H
